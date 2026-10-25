#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShooterWaveTypes.generated.h"

class AShooterEnemy;

/**
 * 웨이브에 섞어 넣을 적 종류 1개와 그 비중.
 * 예: { 기본 적, 2 }, { 빠른 적, 2 }, { 탱커, 1 } → 스폰할 때마다 2:2:1 확률로 고른다
 */
USTRUCT(BlueprintType)
struct GAME_PROJECT_API FShooterWaveEnemyEntry
{
	GENERATED_BODY()

	// 스폰할 적 BP (BP_Enemy, BP_Enemy_Fast 등)
	// EditAnywhere      : DataTable 편집 화면에서 값 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Wave" 그룹으로 표시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TSubclassOf<AShooterEnemy> EnemyClass;

	// 뽑힐 비중. 다른 항목 비중과의 "비율"만 의미 있다 (2:1이면 2/3, 1/3 확률)
	// (지정자 의미는 위와 동일) meta=(ClampMin) : 0 미만 입력 방지 (0이면 안 뽑힘)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0"))
	float Weight = 1.f;
};

/**
 * 웨이브 DataTable(DT_Waves)의 "한 줄" 구조.
 * DataTable 에셋을 만들 때 이 구조체를 Row Structure로 고르면, 아래 변수들이 표의 "열"이 된다.
 * (Unity로 치면 [System.Serializable] class WaveData { ... }를 배열로 들고 있는 ScriptableObject와 비슷)
 *
 * FTableRowBase를 상속해야 DataTable의 행으로 쓸 수 있다.
 * USTRUCT(BlueprintType) : 블루프린트에서도 이 구조체를 변수/핀 타입으로 쓸 수 있게 노출
 */
USTRUCT(BlueprintType)
struct GAME_PROJECT_API FShooterWaveRow : public FTableRowBase
{
	GENERATED_BODY()

	// 이 웨이브에 등장하는 적 수
	// EditAnywhere      : DataTable 편집 화면에서 값 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Wave" 그룹으로 표시
	// meta=(ClampMin)   : 1 미만 입력 방지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "1"))
	int32 EnemyCount = 3;

	// 적을 몇 초마다 한 마리씩 스폰할지 (지정자 의미는 위와 동일, 0.05초 미만 입력 방지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.05"))
	float SpawnInterval = 1.f;

	// 적 종류가 하나뿐일 때 쓰는 기본 적. EnemyMix가 비어 있으면 이 적만 나온다
	// (지정자 의미는 위와 동일)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TSubclassOf<AShooterEnemy> EnemyClass;

	// 여러 종류를 섞을 때의 구성표. 채워져 있으면 EnemyClass 대신 이 목록에서 비중대로 랜덤 선택
	// (지정자 의미는 위와 동일)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<FShooterWaveEnemyEntry> EnemyMix;
};
