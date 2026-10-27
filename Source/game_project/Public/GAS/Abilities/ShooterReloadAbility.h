#pragma once

#include "CoreMinimal.h"
#include "GAS/ShooterGameplayAbility.h"
#include "ShooterReloadAbility.generated.h"

/**
 * 재장전 어빌리티.
 * 들고 있는 무기의 ReloadMontage를 재생하고, 끝까지 재생되면 Ammo를 MaxAmmo로 채운다.
 * 중간에 끊기면(무기 교체, 사망 등) 탄약은 그대로 → "재장전 캔슬"이 자연스럽게 된다.
 *
 * BP(GA_Reload)에서 지정할 것:
 *  - Activation Owned Tags   : State.Reloading (재장전하는 동안 붙는 태그 → GA_Fire가 이 태그로 막힌다)
 *  - Activation Blocked Tags : State.Equipping, State.Dead (교체 중·죽은 뒤에는 재장전 불가)
 */
UCLASS()
class GAME_PROJECT_API UShooterReloadAbility : public UShooterGameplayAbility
{
	GENERATED_BODY()

protected:
	// 실행 가능한지 GAS가 묻는 함수. 부모 검사(태그, 쿨타임 등) + "탄창이 가득 차 있지 않을 것"
	// (Unity로 치면 if (ammo == maxAmmo) return; 를 어빌리티 시스템 차원에서 하는 것)
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 어빌리티가 실행될 때 GAS가 호출하는 함수 (BP의 Event ActivateAbility에 해당)
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	// 재장전 Montage가 끝까지 재생됨 → 탄창 채우고 종료
	// UFUNCTION() : AbilityTask의 이벤트는 다이내믹 델리게이트라서, 연결할 함수가 리플렉션에 등록돼 있어야 한다
	UFUNCTION()
	void OnReloadFinished();

	// 재장전 Montage가 중간에 끊김(다른 Montage가 덮어씀 / 어빌리티 취소) → 탄창 채우지 않고 종료
	// UFUNCTION() : 위와 동일
	UFUNCTION()
	void OnReloadInterrupted();

	// Ammo를 MaxAmmo로 채운다
	void RefillAmmo() const;
};
