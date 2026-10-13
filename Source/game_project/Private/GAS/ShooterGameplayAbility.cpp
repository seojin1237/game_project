#include "GAS/ShooterGameplayAbility.h"

UShooterGameplayAbility::UShooterGameplayAbility()
{
	// 캐릭터마다 어빌리티 객체를 1개 만들어 재사용 → 어빌리티 안에 변수(상태)를 저장할 수 있다
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
