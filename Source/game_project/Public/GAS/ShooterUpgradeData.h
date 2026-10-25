#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShooterUpgradeData.generated.h"

class UGameplayEffect;
class UTexture2D;

/**
 * 업그레이드 카드 1장의 데이터 (DA_Upgrade_XXX 에셋으로 만든다).
 *
 * DataAsset = 데이터만 담는 에셋. Unity의 ScriptableObject와 거의 같은 개념.
 * C++에서는 "어떤 칸이 있는지"만 정하고, 실제 카드 내용은 에디터에서 에셋을 여러 개 만들어 채운다.
 * → 새 업그레이드 추가 = GE 1개 + 이 DataAsset 1개 (C++ 수정 불필요)
 *
 * UPrimaryDataAsset : 나중에 에셋 매니저로 "모든 업그레이드 목록"을 자동 수집할 수 있는 DataAsset 종류
 * BlueprintType     : 블루프린트에서 이 타입을 변수/핀으로 쓸 수 있게 노출 (카드 UI에서 사용)
 */
UCLASS(BlueprintType)
class GAME_PROJECT_API UShooterUpgradeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 카드 제목 ("공격력 강화")
	// EditDefaultsOnly  : DataAsset 에셋 편집 화면에서 값 수정 가능 (에셋 자체가 "기본값"이므로)
	// BlueprintReadOnly : 블루프린트(카드 UI)에서 읽기만 가능
	// Category          : "Upgrade" 그룹으로 표시
	// FText             : 화면에 보여줄 문자열 타입 (나중에 다국어 번역을 지원). 내부 식별용은 FName/FString을 쓴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade")
	FText DisplayName;

	// 카드 설명 ("공격력 +25%") (지정자 의미는 위와 동일)
	// meta=(MultiLine) : 에디터 입력칸을 여러 줄로 (긴 설명 작성용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade", meta = (MultiLine = true))
	FText Description;

	// 카드 아이콘. 지금은 비워두고 아트 에셋이 생기면 지정 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade")
	TObjectPtr<UTexture2D> Icon;

	// 선택했을 때 플레이어에게 적용할 GameplayEffect (보통 Infinite = 영구 버프) (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade")
	TSubclassOf<UGameplayEffect> Effect;

	// [선택] Effect와 함께 추가로 적용할 GE들 (Effect 다음에 순서대로 적용)
	// 예: "최대 체력 +25" (Infinite) + "체력 25 회복" (Instant) 처럼 성격이 다른 효과를 한 카드에 묶을 때
	// (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalEffects;
};
