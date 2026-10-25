#include "Game/ShooterGameMode.h"
#include "Game/ShooterPlayerController.h"
#include "Game/ShooterWaveTypes.h"
#include "Character/ShooterCharacter.h"
#include "Character/ShooterEnemy.h"
#include "Engine/DataTable.h"
#include "GAS/ShooterUpgradeData.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
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

	// 1) 레벨에 놓인 모든 TargetPoint를 스폰 지점으로 수집
	//    GetAllActorsOfClass: 월드에서 해당 클래스 액터를 전부 찾는다 (Unity의 FindObjectsOfType<T>()와 같음)
	//    매 스폰마다 찾으면 느리므로 시작할 때 한 번만 찾아서 저장
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(this, ATargetPoint::StaticClass(), FoundPoints);
	SpawnPoints.Append(FoundPoints);

	if (SpawnPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Wave] 레벨에 TargetPoint가 없습니다. 적을 스폰할 수 없습니다."));
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

	UE_LOG(LogTemp, Log, TEXT("[Wave] %d웨이브 시작 - 적 %d마리"), CurrentWave, EnemiesLeftToSpawn);
	BroadcastWaveState();

	// SpawnInterval초마다 SpawnEnemy 반복. 첫 스폰은 바로(지연 0)
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AShooterGameMode::SpawnEnemy, Row->SpawnInterval, true, 0.f);
}

void AShooterGameMode::SpawnEnemy()
{
	// 다 스폰했거나, 설정이 비었거나, 플레이어가 죽었으면 스폰 중단
	if (EnemiesLeftToSpawn <= 0 || !CurrentEnemyClass || SpawnPoints.IsEmpty() || IsPlayerDead())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	// 스폰 지점 중 하나를 랜덤으로 고른다 (Unity의 Random.Range(0, count))
	const AActor* Point = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];
	if (!Point)
	{
		return;
	}

	// 스폰 옵션: 그 자리에 다른 것이 있으면 살짝 옮겨서라도 반드시 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// SpawnActor<타입>(클래스, 위치, 회전, 옵션): 게임 중에 액터 생성 (Unity의 Instantiate(prefab, pos, rot))
	// 적은 AutoPossessAI = PlacedInWorldOrSpawned 설정이라 스폰되자마자 AI 컨트롤러가 자동으로 붙는다
	AShooterEnemy* Enemy = GetWorld()->SpawnActor<AShooterEnemy>(
		CurrentEnemyClass, Point->GetActorLocation(), Point->GetActorRotation(), Params);

	if (Enemy)
	{
		// 이 적이 죽으면 HandleEnemyDied를 불러달라고 구독 (C#의 enemy.OnDied += HandleEnemyDied)
		// AddDynamic: 다이내믹 델리게이트용 등록 매크로. 연결할 함수는 UFUNCTION()이어야 한다
		Enemy->OnCharacterDied.AddDynamic(this, &AShooterGameMode::HandleEnemyDied);
		++EnemiesAlive;
	}

	--EnemiesLeftToSpawn;
	BroadcastWaveState();
}

void AShooterGameMode::HandleEnemyDied(AShooterCharacterBase* DeadCharacter)
{
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);
	BroadcastWaveState();

	// 살아있는 적도, 스폰 대기 중인 적도 없으면 웨이브 클리어
	if (EnemiesAlive == 0 && EnemiesLeftToSpawn == 0 && !IsPlayerDead())
	{
		UE_LOG(LogTemp, Log, TEXT("[Wave] %d웨이브 클리어!"), CurrentWave);

		// 업그레이드 선택 화면을 띄운다. 띄우지 못하면(풀이 비었거나 등) 바로 다음 웨이브 예약
		// 띄웠다면 플레이어가 카드를 고를 때 NotifyUpgradeChosen()이 호출되어 이어서 진행된다
		if (!OfferUpgrades())
		{
			ScheduleNextWave();
		}
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
