#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UDataTable;
class AShooterEnemy;
class AShooterCharacterBase;
class UShooterUpgradeData;

/**
 * 웨이브 상태가 바뀔 때(웨이브 시작, 적 스폰, 적 사망) 알려주는 이벤트 타입.
 * DYNAMIC_MULTICAST : 여러 구독자를 가질 수 있고(C#의 event), 블루프린트에서도 바인딩 가능한 델리게이트
 * TwoParams         : 매개변수 2개 (타입, 이름) 쌍으로 적는다
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStateChanged, int32, WaveNumber, int32, EnemiesRemaining);

/**
 * 게임 규칙 담당: 웨이브 진행.
 *  1) 시작 후 FirstWaveDelay초 대기 → 1웨이브 시작
 *  2) 웨이브 데이터(DT_Waves)의 적 수만큼, SpawnInterval초마다 1마리씩 스폰 (레벨의 TargetPoint 중 랜덤 위치)
 *  3) 그 웨이브의 적을 모두 처치하면 TimeBetweenWaves초 쉬고 다음 웨이브
 *  4) 표의 마지막 줄을 넘어가면 마지막 줄을 재사용하되 웨이브마다 적을 더 늘린다 (끝없는 생존)
 *  5) 플레이어가 죽으면 진행 중단
 */
UCLASS()
class GAME_PROJECT_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShooterGameMode();

	// 웨이브 상태 변경 알림. HUD 등이 구독한다
	// BlueprintAssignable : 블루프린트에서 이 이벤트에 "Bind Event"로 연결할 수 있게 노출
	// Category            : "Wave" 그룹으로 표시
	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveStateChanged OnWaveStateChanged;

	// 현재 웨이브 번호 (시작 전이면 0)
	// BlueprintPure : 실행 핀 없는 값 읽기 전용 BP 노드
	// Category      : "Wave" 그룹으로 표시
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetCurrentWave() const { return CurrentWave; }

	// 이번 웨이브에 남은 적 수 = 살아있는 적 + 아직 스폰 안 된 적 (지정자 의미는 위와 동일)
	UFUNCTION(BlueprintPure, Category = "Wave")
	int32 GetEnemiesRemaining() const { return EnemiesAlive + EnemiesLeftToSpawn; }

	// 업그레이드 후보 목록 (G-3에서 여기서 카드 3장을 뽑는다)
	// BlueprintPure : 값 읽기 전용 BP 노드 / Category : "Upgrade" 그룹으로 표시
	// ObjectPtrDecay: TArray<TObjectPtr<T>> 를 일반 포인터 배열 TArray<T*> 로 복사 없이 바꿔 보여준다
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	const TArray<UShooterUpgradeData*>& GetUpgradePool() const { return ObjectPtrDecay(UpgradePool); }

	// [테스트용] 콘솔(~)에 "DebugApplyUpgrade 0" 처럼 입력 → UpgradePool[0]을 플레이어에게 적용
	// Exec : 콘솔 명령어로 호출 가능 (GameMode는 Exec가 동작하는 클래스 중 하나)
	UFUNCTION(Exec)
	void DebugApplyUpgrade(int32 Index);

protected:
	// 게임 시작 시 호출 (Unity의 Start) → 스폰 지점 수집, 첫 웨이브 예약
	virtual void BeginPlay() override;

	// 다음 웨이브 시작: 표에서 데이터를 읽고 스폰 타이머 시작
	void StartNextWave();

	// 스폰 타이머가 주기적으로 호출: 적 1마리 스폰
	void SpawnEnemy();

	// 스폰한 적이 죽었을 때 호출 (적의 OnCharacterDied 이벤트에 연결)
	// UFUNCTION() : 다이내믹 델리게이트(AddDynamic)에 연결하려면 리플렉션에 등록된 함수여야 해서 필요
	UFUNCTION()
	void HandleEnemyDied(AShooterCharacterBase* DeadCharacter);

	// 현재 상태를 OnWaveStateChanged로 알림
	void BroadcastWaveState();

	// 플레이어가 죽었는지 (죽었으면 웨이브 진행 중단)
	bool IsPlayerDead() const;

	// 웨이브 구성 표 (BP에서 DT_Waves 지정, Row Structure는 FShooterWaveRow)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Wave" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	TObjectPtr<UDataTable> WaveTable;

	// 게임 시작 후 첫 웨이브까지 대기 시간(초)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Wave" 그룹으로 표시
	// meta=(ClampMin)  : 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Wave", meta = (ClampMin = "0"))
	float FirstWaveDelay = 3.f;

	// 웨이브 클리어 후 다음 웨이브까지 쉬는 시간(초) (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Wave", meta = (ClampMin = "0"))
	float TimeBetweenWaves = 5.f;

	// 표의 마지막 줄을 넘어간 웨이브부터, 한 웨이브마다 추가로 늘어나는 적 수 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Wave", meta = (ClampMin = "0"))
	int32 ExtraEnemiesPerWaveAfterTable = 2;

	// 등장 가능한 업그레이드 카드 목록 (BP에서 DA_Upgrade_XXX 에셋들을 추가)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Upgrade" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	TArray<TObjectPtr<UShooterUpgradeData>> UpgradePool;

private:
	// ===== 진행 상태 =====
	int32 CurrentWave = 0;          // 현재 웨이브 번호 (1부터)
	int32 EnemiesLeftToSpawn = 0;   // 이번 웨이브에서 아직 스폰 안 된 적 수
	int32 EnemiesAlive = 0;         // 스폰되어 살아있는 적 수

	// 이번 웨이브 설정 (표에서 읽어온 값)
	UPROPERTY()
	TSubclassOf<AShooterEnemy> CurrentEnemyClass;

	// 레벨에 배치된 스폰 지점(TargetPoint)들
	// UPROPERTY() : 가비지 컬렉터가 추적하도록 (UObject 포인터 배열 멤버엔 필수)
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnPoints;

	// 타이머 번호표 (다음 웨이브 예약용 / 적 스폰 반복용)
	FTimerHandle WaveTimerHandle;
	FTimerHandle SpawnTimerHandle;
};
