#include "GAS/ShooterAbilitySystemGlobals.h"
#include "GAS/ShooterGameplayEffectContext.h"

FGameplayEffectContext* UShooterAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	// 기본 FGameplayEffectContext 대신 치명타 정보를 담을 수 있는 우리 Context를 만든다
	return new FShooterGameplayEffectContext();
}
