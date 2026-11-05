#include "Game/ShooterGameMode.h"
#include "Game/ShooterPlayerController.h"
#include "Game/ShooterWaveTypes.h"
#include "Character/ShooterCharacter.h"
#include "Character/ShooterEnemy.h"
#include "Engine/DataTable.h"
#include "GAS/ShooterUpgradeData.h"
#include "GAS/ShooterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

AShooterGameMode::AShooterGameMode()
{
	// C++ 기본값. 실제로는 BP_ShooterGameMode에서 BP_ShooterCharacter로 바꿔서 사용
	DefaultPawnClass = AShooterCharacter::StaticClass();

	// 플레이어가 접속하면 만들어질 컨트롤러 클래스. 실제로는 BP에서 BP_ShooterPlayerController로 바꿔서 사용
	PlayerControllerClass = AShooterPlayerController::StaticClass();
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 첫 스폰 방향을 랜덤으로 정한다. 이후 방향은 FindRandomSpawnTransform이 돌려 나간다
	NextSpawnAngle = FMath::FRandRange(0.f, 360.f);

	// 1) 레벨에 놓인 모든 TargetPoint를 예비 스폰 지점으로 수집 (랜덤 위치를 찾지 못했을 때만 쓴다)
	//    GetAllActorsOfClass: 월드에서 해당 클래스 액터를 전부 찾는다 (Unity의 FindObjectsOfType<T>()와 같음)
	//    매 스폰마다 찾으면 느리므로 시작할 때 한 번만 찾아서 저장
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(this, ATargetPoint::StaticClass(), FoundPoints);
	SpawnPoints.Append(FoundPoints);

	if (SpawnPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Wave] 레벨에 TargetPoint가 없습니다. 랜덤 위치를 찾지 못하면 적을 스폰할 수 없습니다."));
	}
	if (!WaveTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Wave] WaveTable이 비어 있습니다. BP_ShooterGameMode에서 DT_Waves를 지정하세요."));
		return;
	}

	// 2) FirstWaveDelay초 뒤에 첫 웨이브 시작 (반복 없음)
	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AShooterGameMode::StartNextWave, FirstWaveDelay, false);

	// 시작 전 상태(웨이브 0)도 한 번 알려줘서 HUD가 "준비" 상태를 표시할 수 있게
	BroadcastWaveState();
}

void AShooterGameMode::StartNextWave()
{
	if (IsPlayerDead() || !WaveTable)
	{
		return;
	}

	// GetRowNames: 표의 행 이름 목록 (에디터에 보이는 순서). 행 이름이 아니라 "순서"로 웨이브를 정한다
	const TArray<FName> RowNames = WaveTable->GetRowNames();
	if (RowNames.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Wave] WaveTable에 행이 없습니다."));
		return;
	}

	++CurrentWave;

	// 표보다 웨이브가 길어지면 마지막 줄을 계속 사용 (Min으로 인덱스를 마지막 줄에 고정)
	const int32 RowIndex = FMath::Min(CurrentWave - 1, RowNames.Num() - 1);

	// FindRow<구조체>(행 이름, 에러 시 로그에 찍힐 문맥 문자열): 그 행의 데이터를 구조체 포인터로 얻는다
	const FShooterWaveRow* Row = WaveTable->FindRow<FShooterWaveRow>(RowNames[RowIndex], TEXT("StartNextWave"));
	if (!Row)
	{
		return;
	}

	// 마지막 줄을 넘어선 웨이브 수만큼 적을 추가 (예: 표가 3줄이면 5웨이브 = 마지막 줄 + 2 * Extra)
	const int32 WavesBeyondTable = FMath::Max(0, CurrentWave - RowNames.Num());
	EnemiesLeftToSpawn = Row->EnemyCount + WavesBeyondTable * ExtraEnemiesPerWaveAfterTable;
	CurrentEnemyClass = Row->EnemyClass;
	CurrentEnemyMix = Row->EnemyMix;

	// 보스 웨이브면 평소 적에 보스 1마리를 더한다. %: 나머지 연산 (C#과 같음)
	bBossPendingSpawn = BossClass && CurrentWave % BossWaveInterval == 0;
	if (bBossPendingSpawn)
	{
		++EnemiesLeftToSpawn;
	}

	UE_LOG(LogTemp, Log, TEXT("[Wave] %d웨이브 시작 - 적 %d마리%s"), CurrentWave, EnemiesLeftToSpawn, bBossPendingSpawn ? TEXT(" (보스 포함)") : TEXT(""));
	BroadcastWaveState();

	// SpawnInterval초마다 SpawnEnemy 반복. 첫 스폰은 바로(지연 0)
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AShooterGameMode::SpawnEnemy, Row->SpawnInterval, true, 0.f);
}

void AShooterGameMode::SpawnEnemy()
{
	// 이번에 스폰할 적 종류 결정. 보스가 남아 있으면 보스부터, 아니면 구성표의 비중대로 랜덤
	const TSubclassOf<AShooterEnemy> EnemyClass = bBossPendingSpawn ? BossClass : PickEnemyClass();

	// 다 스폰했거나, 설정이 비었거나, 플레이어가 죽었으면 스폰 중단
	if (EnemiesLeftToSpawn <= 0 || !EnemyClass || IsPlayerDead())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	FVector SpawnLocation;
	FRotator SpawnRotation;
	if (!FindRandomSpawnTransform(EnemyClass, SpawnLocation, SpawnRotation))
	{
		// 랜덤 위치를 못 찾으면 TargetPoint 중 하나를 쓴다 (Unity의 Random.Range(0, count))
		const AActor* Point = SpawnPoints.IsEmpty() ? nullptr : SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)].Get();
		if (!Point)
		{
			// 타이머는 그대로 두어 다음 간격에 다시 시도한다 (플레이어가 움직이면 찾을 수 있다)
			UE_LOG(LogTemp, Warning, TEXT("[Wave] 스폰 위치를 찾지 못했습니다. 다음 간격에 다시 시도합니다."));
			return;
		}
		SpawnLocation = Point->GetActorLocation();
		SpawnRotation = Point->GetActorRotation();
		UE_LOG(LogTemp, Warning, TEXT("[Wave] 랜덤 스폰 위치를 찾지 못해 TargetPoint를 사용합니다."));
	}

	// 스폰 옵션: 그 자리에 다른 것이 있으면 살짝 옮겨서라도 반드시 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// SpawnActor<타입>(클래스, 위치, 회전, 옵션): 게임 중에 액터 생성 (Unity의 Instantiate(prefab, pos, rot))
	// 적은 AutoPossessAI = PlacedInWorldOrSpawned 설정이라 스폰되자마자 AI 컨트롤러가 자동으로 붙는다
	AShooterEnemy* Enemy = GetWorld()->SpawnActor<AShooterEnemy>(
		EnemyClass, SpawnLocation, SpawnRotation, Params);

	if (Enemy)
	{
		// 이 적이 죽으면 HandleEnemyDied를 불러달라고 구독 (C#의 enemy.OnDied += HandleEnemyDied)
		// AddDynamic: 다이내믹 델리게이트용 등록 매크로. 연결할 함수는 UFUNCTION()이어야 한다
		Enemy->OnCharacterDied.AddDynamic(this, &AShooterGameMode::HandleEnemyDied);
		++EnemiesAlive;

		// 보스를 잡아야 웨이브가 끝나므로, 지금까지 넘긴 보스 웨이브 수 = 잡은 보스 수다. 따로 세지 않아도 웨이브 번호로 알 수 있다
		// 웨이브 시작 때 정해지는 값이라, 보스가 죽어도 진행 중인 웨이브의 남은 적은 강해지지 않고 다음 웨이브부터 강해진다
		// 1번 적용할 때마다 GE의 배율이 곱해지므로 보스를 잡을수록 복리로 강해진다
		const int32 BossesDefeated = BossClass ? (CurrentWave - 1) / BossWaveInterval : 0;
		UAbilitySystemComponent* EnemyASC = Enemy->GetAbilitySystemComponent();
		if (BossDefeatBuffEffect && EnemyASC && BossesDefeated > 0)
		{
			const FGameplayEffectSpecHandle Spec = EnemyASC->MakeOutgoingSpec(BossDefeatBuffEffect, 1.f, EnemyASC->MakeEffectContext());
			for (int32 Count = 0; Spec.IsValid() && Count < BossesDefeated; ++Count)
			{
				EnemyASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	bBossPendingSpawn = false;
	--EnemiesLeftToSpawn;
	BroadcastWaveState();
}

bool AShooterGameMode::FindRandomSpawnTransform(TSubclassOf<AShooterEnemy> EnemyClass, FVector& OutLocation, FRotator& OutRotation)
{
	// UNavigationSystemV1: 레벨의 NavMesh(AI가 걸어 다닐 수 있는 영역)를 다루는 시스템 (Unity의 NavMesh 클래스)
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Player || !NavSystem)
	{
		return false;
	}

	const FVector PlayerLocation = Player->GetActorLocation();
	const float HalfHeight = EnemyClass->GetDefaultObject<AShooterEnemy>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 맵 밖이나 벽 안처럼 못 쓰는 자리가 나올 수 있으므로 몇 번 다시 뽑는다
	constexpr int32 MaxAttempts = 8;
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// 방향: 황금각(137.5도)씩 돌리면 몇 마리를 뽑아도 방향이 고르게 나뉜다. 규칙적으로 보이지 않게 흔들어 준다
		NextSpawnAngle = FMath::Fmod(NextSpawnAngle + 137.5f + FMath::FRandRange(-20.f, 20.f), 360.f);

		// 거리: 그냥 랜덤으로 뽑으면 면적이 좁은 안쪽에 몰린다. 제곱을 랜덤으로 뽑고 제곱근을 구해야 면적 기준으로 고르게 퍼진다
		const float Radius = FMath::Sqrt(FMath::Lerp(FMath::Square(SpawnMinRadius), FMath::Square(SpawnMaxRadius), FMath::FRand()));
		const FVector Direction = FRotator(0.f, NextSpawnAngle, 0.f).Vector();

		// ProjectPointToNavigation: 그 지점 근처의 NavMesh 위 좌표를 찾는다. 위아래로 넉넉히 찾아 플레이어와 높이가 달라도 찾게 한다
		FNavLocation NavLocation;
		if (!NavSystem->ProjectPointToNavigation(PlayerLocation + Direction * Radius, NavLocation, FVector(100.f, 100.f, 500.f)))
		{
			continue;
		}

		// 플레이어까지 길이 끊겨 있으면(장애물 위 등) 적이 오지 못해 웨이브가 끝나지 않으므로 끝까지 이어지는 길만 쓴다
		const UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(this, NavLocation.Location, PlayerLocation);
		if (!Path || !Path->IsValid() || Path->IsPartial())
		{
			continue;
		}

		// NavMesh 좌표는 바닥이고 액터 위치는 캡슐 중심이므로 캡슐 절반만큼 올린다. 회전은 플레이어를 보는 좌우 방향만 쓴다
		OutLocation = NavLocation.Location + FVector(0.f, 0.f, HalfHeight);
		OutRotation = FRotator(0.f, (PlayerLocation - OutLocation).Rotation().Yaw, 0.f);
		UE_LOG(LogTemp, Log, TEXT("[Wave] 스폰: 방향 %.0f도, 거리 %.0f"), NextSpawnAngle, FVector::Dist2D(OutLocation, PlayerLocation));
		return true;
	}

	return false;
}

TSubclassOf<AShooterEnemy> AShooterGameMode::PickEnemyClass() const
{
	// 1) 비중 합계 구하기 (비어 있는 칸, 비중 0인 칸은 제외)
	float TotalWeight = 0.f;
	for (const FShooterWaveEnemyEntry& Entry : CurrentEnemyMix)
	{
		if (Entry.EnemyClass && Entry.Weight > 0.f)
		{
			TotalWeight += Entry.Weight;
		}
	}

	// 구성표가 비었으면 기본 적 하나만 사용
	if (TotalWeight <= 0.f)
	{
		return CurrentEnemyClass;
	}

	// 2) 가중치 랜덤 (룰렛 방식): 0~합계 사이 숫자를 뽑고, 비중을 차례로 빼다가 0 이하가 되는 칸을 선택
	//    예: 비중 2,2,1 → 0~5 중 Roll=3.4 → 3.4-2=1.4 → 1.4-2=-0.6 → 두 번째 칸 당첨
	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (const FShooterWaveEnemyEntry& Entry : CurrentEnemyMix)
	{
		if (!Entry.EnemyClass || Entry.Weight <= 0.f)
		{
			continue;
		}
		Roll -= Entry.Weight;
		if (Roll <= 0.f)
		{
			return Entry.EnemyClass;
		}
	}

	// 부동소수점 오차로 루프를 빠져나온 경우 대비: 마지막 유효 항목 반환
	for (int32 i = CurrentEnemyMix.Num() - 1; i >= 0; --i)
	{
		if (CurrentEnemyMix[i].EnemyClass && CurrentEnemyMix[i].Weight > 0.f)
		{
			return CurrentEnemyMix[i].EnemyClass;
		}
	}
	return CurrentEnemyClass;
}

void AShooterGameMode::HandleEnemyDied(AShooterCharacterBase* DeadCharacter)
{
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);

	// 흡혈 업그레이드: 플레이어의 KillHeal만큼 회복 (기본 0이라 업그레이드가 없으면 아무 일 없음)
	// (싱글플레이라 적을 죽이는 건 항상 플레이어 → 누가 죽였는지 따로 확인하지 않는다)
	if (AShooterCharacterBase* Player = Cast<AShooterCharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent())
		{
			const float KillHeal = PlayerASC->GetNumericAttribute(UShooterAttributeSet::GetKillHealAttribute());
			Player->Heal(KillHeal);
		}
	}

	BroadcastWaveState();

	// 살아있는 적도, 스폰 대기 중인 적도 없으면 웨이브 클리어
	if (EnemiesAlive == 0 && EnemiesLeftToSpawn == 0 && !IsPlayerDead())
	{
		UE_LOG(LogTemp, Log, TEXT("[Wave] %d웨이브 클리어!"), CurrentWave);

		// 바로 화면을 띄우면 마지막 적이 쓰러지는 모습이 멈춰 버리므로 잠깐 기다린다
		GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AShooterGameMode::FinishWaveClear, UpgradeOfferDelay, false);
	}
}

void AShooterGameMode::FinishWaveClear()
{
	// 기다리는 사이에 플레이어가 죽었으면 게임 오버 화면만 나와야 한다
	if (IsPlayerDead())
	{
		return;
	}

	// 업그레이드 선택 화면을 띄운다. 띄우지 못하면(풀이 비었거나 등) 바로 다음 웨이브 예약
	// 띄웠다면 플레이어가 카드를 고를 때 NotifyUpgradeChosen()이 호출되어 이어서 진행된다
	if (!OfferUpgrades())
	{
		ScheduleNextWave();
	}
}

bool AShooterGameMode::OfferUpgrades()
{
	// 비어 있는 칸(None)은 빼고 후보 목록을 만든다
	TArray<UShooterUpgradeData*> Candidates;
	for (UShooterUpgradeData* Upgrade : UpgradePool)
	{
		if (Upgrade)
		{
			Candidates.Add(Upgrade);
		}
	}
	if (Candidates.IsEmpty())
	{
		return false;
	}

	// 섞기 (Fisher-Yates 셔플): 뒤에서부터 하나씩, 자기 자신 이하 인덱스 중 랜덤 위치와 교환
	// → 중복 없이 랜덤 N장을 뽑기 위해 섞은 뒤 앞에서 N개를 쓴다
	for (int32 i = Candidates.Num() - 1; i > 0; --i)
	{
		Candidates.Swap(i, FMath::RandRange(0, i));
	}
	// SetNum: 배열 길이를 N으로 자른다 (풀이 N보다 적으면 그대로)
	Candidates.SetNum(FMath::Min(UpgradeChoiceCount, Candidates.Num()));

	// 0번 플레이어의 컨트롤러에게 선택 화면을 띄우라고 요청
	AShooterPlayerController* PC = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!PC)
	{
		return false;
	}
	PC->ShowUpgradeSelection(Candidates);
	return true;
}

void AShooterGameMode::NotifyUpgradeChosen()
{
	ScheduleNextWave();
}

void AShooterGameMode::ScheduleNextWave()
{
	UE_LOG(LogTemp, Log, TEXT("[Wave] %.0f초 후 다음 웨이브"), TimeBetweenWaves);
	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AShooterGameMode::StartNextWave, TimeBetweenWaves, false);
}

void AShooterGameMode::BroadcastWaveState()
{
	// 구독한 모든 대상(HUD 등)에게 알림 (C#의 OnWaveStateChanged?.Invoke(wave, remaining))
	OnWaveStateChanged.Broadcast(CurrentWave, GetEnemiesRemaining());
}

void AShooterGameMode::DebugSkipToWave(int32 Wave)
{
	// StartNextWave가 번호를 1 올리므로 1 작게 넣는다. 예약된 다음 웨이브는 취소해 두 웨이브가 겹치지 않게 한다
	CurrentWave = FMath::Max(0, Wave - 1);
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	StartNextWave();
}

void AShooterGameMode::DebugApplyUpgrade(int32 Index)
{
	// IsValidIndex: 배열 범위 안인지 확인 (C#에서 index < list.Count 체크와 같음)
	if (!UpgradePool.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Upgrade] 잘못된 번호 %d (UpgradePool 개수: %d)"), Index, UpgradePool.Num());
		return;
	}

	if (AShooterCharacterBase* Player = Cast<AShooterCharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->ApplyUpgrade(UpgradePool[Index]);
	}
}

bool AShooterGameMode::IsPlayerDead() const
{
	const AShooterCharacterBase* Player = Cast<AShooterCharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	return Player && Player->IsDead();
}
