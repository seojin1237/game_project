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

	// 공격력: 이 캐릭터가 GE_Damage를 적용할 때 대상 체력에서 빠지는 양
	// (GE_Damage의 Magnitude가 AttributeBased로 "공격자(Source)의 AttackPower"를 읽는다)
	// 업그레이드 예: "공격력 +20%" = AttackPower에 1.2를 곱하는 Infinite GE (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS_BASIC(UShooterAttributeSet, AttackPower)

	// 사격 간격(초). 작을수록 빨리 쏜다
	// (GE_Cooldown_Fire의 Duration이 AttributeBased로 이 값을 읽는다) (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData FireInterval;
	ATTRIBUTE_ACCESSORS_BASIC(UShooterAttributeSet, FireInterval)

	// 이동 속도(cm/s). 값이 바뀌면 캐릭터가 CharacterMovement의 MaxWalkSpeed에 반영한다
	// (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UShooterAttributeSet, MoveSpeed)

	// 적을 처치할 때마다 회복하는 체력 (기본 0 = 효과 없음). "흡혈" 업그레이드가 올려준다
	// (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData KillHeal;
	ATTRIBUTE_ACCESSORS_BASIC(UShooterAttributeSet, KillHeal)
};
