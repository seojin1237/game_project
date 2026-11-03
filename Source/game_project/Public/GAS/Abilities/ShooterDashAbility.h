#pragma once

#include "CoreMinimal.h"
#include "GAS/ShooterGameplayAbility.h"
#include "ShooterDashAbility.generated.h"

/**
 * 대시 어빌리티.
 * 누르고 있는 이동 방향으로 짧은 시간 동안 일정한 거리를 밀어 준다. 이동 입력이 없으면 몸이 보는 방향으로 밀어 준다.
 *
 * BP(GA_Dash)에서 지정할 것:
 *  - Cooldown Gameplay Effect Class : GE_Cooldown_Dash (쿨타임 시간은 이 GE에서 조절)
 *  - Activation Owned Tags          : State.Dashing (대시하는 동안 붙는 태그 → GA_Fire가 이 태그로 막힌다)
 *  - Activation Blocked Tags        : State.Dead, State.InAir (죽은 뒤와 공중에서는 대시 불가)
 */
UCLASS()
class GAME_PROJECT_API UShooterDashAbility : public UShooterGameplayAbility
{
	GENERATED_BODY()

protected:
	// 어빌리티가 실행될 때 GAS가 호출하는 함수 (BP의 Event ActivateAbility에 해당)
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 대시로 이동할 거리 (cm)
	// EditDefaultsOnly : GA_Dash의 Class Defaults에서만 수정 가능 (손맛 수치라 BP에서 튜닝)
	// Category         : "Dash" 그룹으로 표시
	// meta=(ClampMin)  : 0 이하 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "1"))
	float DashDistance = 500.f;

	// 대시에 걸리는 시간 (초). 짧을수록 빠르게 튀어 나간다 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0.05"))
	float DashDuration = 0.25f;

private:
	// 밀어 주는 시간이 끝남 → 어빌리티 종료
	// UFUNCTION() : AbilityTask의 이벤트는 다이내믹 델리게이트라서, 연결할 함수가 리플렉션에 등록돼 있어야 한다
	UFUNCTION()
	void OnDashFinished();
};
