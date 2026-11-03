#pragma once

#include "CoreMinimal.h"
#include "GAS/ShooterGameplayAbility.h"
#include "ShooterMeleeAbility.generated.h"

class UGameplayEffect;
class UAnimMontage;

/**
 * 근접 공격 어빌리티 (적의 기본 공격용, 플레이어도 쓸 수 있게 일반화).
 *
 * 동작: 내 앞쪽에 구체(Sphere)를 휘둘러서 → 맞은 "상대편" 캐릭터들에게 DamageEffect 적용.
 * "상대편" = 조종 주체가 나와 다른 쪽 (플레이어 ↔ AI). 적끼리는 서로 때리지 않는다.
 *
 * BP(GA_EnemyAttack)에서 데미지 GE, 사거리, 범위, 쿨타임, 공격 몽타주를 지정한다.
 */
UCLASS()
class GAME_PROJECT_API UShooterMeleeAbility : public UShooterGameplayAbility
{
	GENERATED_BODY()

public:
	UShooterMeleeAbility();

protected:
	// 어빌리티가 실행될 때 GAS가 호출하는 함수 (BP의 Event ActivateAbility에 해당)
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 앞쪽을 쓸어서 맞은 상대에게 데미지 적용 (실제 판정)
	void PerformAttack();

	// 태스크 이벤트로 불리는 함수들. 지정자 없는 UFUNCTION(): AddDynamic으로 연결하려면 리플렉션에 등록돼야 한다
	// HitDelay가 지나 판정할 시간이 됨
	UFUNCTION()
	void OnHitTime();

	// 몽타주가 끝나거나 끊김 → 어빌리티 종료
	UFUNCTION()
	void OnAttackFinished();

	// 맞은 대상에게 적용할 데미지 Effect (BP에서 GE_Damage 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Melee" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 판정 구체가 내 몸 중심에서 앞으로 얼마나 나아가는지 (cm)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Melee" 그룹으로 표시
	// meta=(ClampMin)  : 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Melee", meta = (ClampMin = "0"))
	float AttackRange = 150.f;

	// 판정 구체의 반지름 (cm). 클수록 대충 휘둘러도 맞는다 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Melee", meta = (ClampMin = "1"))
	float AttackRadius = 60.f;

	// [선택] 공격 애니메이션 몽타주. 비워두면 애니메이션 없이 바로 판정한다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Melee|Animation" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 몽타주 시작 후 판정까지 걸리는 시간(초). 주먹이 뻗는 순간에 맞춰 조절한다
	// 몽타주 길이보다 길면 판정 전에 어빌리티가 끝나 데미지가 들어가지 않는다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Melee|Animation" 하위 그룹으로 표시
	// meta=(ClampMin)  : 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Animation", meta = (ClampMin = "0"))
	float HitDelay = 0.4f;

	// 판정 범위를 디버그 구체로 그릴지 여부
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Melee|Debug" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Debug")
	bool bDrawDebug = true;
};
