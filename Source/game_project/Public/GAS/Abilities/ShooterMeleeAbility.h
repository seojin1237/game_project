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

	// [선택] 공격 애니메이션 몽타주. 비워두면 애니메이션 없이 판정만 한다
	// → 나중에 캐릭터 에셋을 가져오면 여기에 몽타주만 지정하면 된다 (코드 수정 불필요)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Melee|Animation" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 판정 범위를 디버그 구체로 그릴지 여부
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Melee|Debug" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Debug")
	bool bDrawDebug = true;
};
