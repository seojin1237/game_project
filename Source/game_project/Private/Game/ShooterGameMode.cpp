#include "Game/ShooterGameMode.h"
#include "Game/ShooterPlayerController.h"
#include "Game/ShooterWaveTypes.h"
#include "Character/ShooterCharacter.h"
#include "Character/ShooterEnemy.h"
#include "Engine/DataTable.h"
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

	// 살아있는 적도, 스폰 대기 중인 적도 없으면 웨이브 클리어 → 쉬었다가 다음 웨이브
	if (EnemiesAlive == 0 && EnemiesLeftToSpawn == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Wave] %d웨이브 클리어! %.0f초 후 다음 웨이브"), CurrentWave, TimeBetweenWaves);
		GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AShooterGameMode::StartNextWave, TimeBetweenWaves, false);
	}
}

void AShooterGameMode::BroadcastWaveState()
{
	// 구독한 모든 대상(HUD 등)에게 알림 (C#의 OnWaveStateChanged?.Invoke(wave, remaining))
	OnWaveStateChanged.Broadcast(CurrentWave, GetEnemiesRemaining());
}

bool AShooterGameMode::IsPlayerDead() const
{
	const AShooterCharacterBase* Player = Cast<AShooterCharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	return Player && Player->IsDead();
}
