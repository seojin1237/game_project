#include "GAS/ShooterAttributeSet.h"
#include "GameplayEffectExtension.h" // FGameplayEffectModCallbackData 정의

UShooterAttributeSet::UShooterAttributeSet()
{
	// 임시 초기값. 나중에 초기화용 GameplayEffect(GE_InitStats)로 옮겨서 BP에서 조절할 예정
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void UShooterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// NewValue가 참조(&)라서 여기서 값을 고치면 실제로 적용될 값이 바뀐다
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UShooterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 이번에 바뀐 Attribute가 Health일 때만 처리
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Instant 효과는 PreAttributeChange를 거치지 않으므로 여기서 한 번 더 0 ~ MaxHealth로 제한
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		UE_LOG(LogTemp, Log, TEXT("[GAS] Health: %.1f / %.1f"), GetHealth(), GetMaxHealth());
	}
}
