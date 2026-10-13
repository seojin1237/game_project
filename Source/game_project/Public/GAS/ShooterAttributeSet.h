#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
// ATTRIBUTE_ACCESSORS_BASIC의 Set/Init 함수가 헤더 안에서 ASC 함수를 직접 호출하므로
// 전방 선언(class UAbilitySystemComponent;)만으로는 부족하고 전체 정의가 필요하다
#include "AbilitySystemComponent.h"
#include "ShooterAttributeSet.generated.h"

/**
 * 캐릭터의 숫자 스탯(Attribute)을 모아두는 클래스.
 * GameplayEffect(GE_)는 여기 있는 값들만 변경할 수 있다.
 */
UCLASS()
class GAME_PROJECT_API UShooterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UShooterAttributeSet();

	// CurrentValue가 바뀌기 "직전"에 호출 (지속형 버프/디버프 등) → 값 범위 제한용
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Instant GameplayEffect가 BaseValue를 바꾼 "직후"에 호출 → 데미지 결과 처리용
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// 현재 체력
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능 (값 변경은 반드시 GameplayEffect로)
	// Category          : 블루프린트에서 "Attributes" 그룹으로 표시
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UShooterAttributeSet, Health)

	// 최대 체력 (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UShooterAttributeSet, MaxHealth)
};
