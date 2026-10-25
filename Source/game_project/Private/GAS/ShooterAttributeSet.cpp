#include "GAS/ShooterAttributeSet.h"
#include "GameplayEffectExtension.h" // FGameplayEffectModCallbackData 정의

UShooterAttributeSet::UShooterAttributeSet()
{
	// "안전한 기본값". 실제 초기 스탯은 캐릭터 BP에 지정한 초기화 GE(GE_InitStats_Player/Enemy)가 덮어쓴다.
	// 초기화 GE를 지정하지 않았을 때도 게임이 이상하게 동작하지 않도록 적당한 값을 넣어둔다
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitAttackPower(20.f);
	InitFireInterval(0.2f);
	InitMoveSpeed(600.f);
}

void UShooterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// NewValue가 참조(&)라서 여기서 값을 고치면 실제로 적용될 값이 바뀐다.
	// 업그레이드/디버프가 쌓여도 말이 안 되는 값이 되지 않도록 범위를 제한한다
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);       // 최대 체력 0 이하 방지
	}
	else if (Attribute == GetAttackPowerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);       // 음수 공격력(=회복) 방지
	}
	else if (Attribute == GetFireIntervalAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.05f);     // 연사 속도 상한 (초당 20발)
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);       // 음수 속도 방지
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

		// 플레이어/과녁 등 여러 액터가 같은 AttributeSet을 쓰므로 누구의 체력인지 이름도 출력
		// GetNameSafe: 포인터가 nullptr이어도 크래시 없이 "None"을 돌려준다
		UE_LOG(LogTemp, Log, TEXT("[GAS] %s Health: %.1f / %.1f"),
			*GetNameSafe(GetOwningActor()), GetHealth(), GetMaxHealth());
	}
}
