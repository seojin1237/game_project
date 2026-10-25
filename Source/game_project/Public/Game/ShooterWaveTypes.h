#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShooterWaveTypes.generated.h"

class AShooterEnemy;

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

	// 스폰할 적 종류 (BP_Enemy 등). 나중에 웨이브마다 다른 적을 섞을 때 이 열을 확장한다
	// (지정자 의미는 위와 동일)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TSubclassOf<AShooterEnemy> EnemyClass;
};
