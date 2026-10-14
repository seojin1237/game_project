#pragma once

#include "CoreMinimal.h"
#include "GAS/ShooterGameplayAbility.h"
#include "ShooterFireAbility.generated.h"

class UGameplayEffect;

/**
 * 히트스캔 사격 어빌리티.
 * 화면 중앙(카메라)에서 앞으로 라인 트레이스 → 맞은 대상에 ASC가 있으면 DamageEffect 적용.
 * BP(GA_Fire)는 이 클래스를 부모로 두고 수치만 조절한다.
 */
UCLASS()
class GAME_PROJECT_API UShooterFireAbility : public UShooterGameplayAbility
{
	GENERATED_BODY()

public:
	UShooterFireAbility();

protected:
	// 어빌리티가 실행될 때 GAS가 호출하는 함수 (BP의 Event ActivateAbility에 해당)
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 맞은 대상에게 적용할 데미지 Effect (BP에서 GE_Damage 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Fire" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 사거리 (cm 단위. 언리얼 1 유닛 = 1cm)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Fire" 그룹으로 표시
	// meta=(ClampMin)  : 에디터에서 0 미만 값을 입력하지 못하게 막음
	UPROPERTY(EditDefaultsOnly, Category = "Fire", meta = (ClampMin = "0"))
	float Range = 5000.f;

	// 총알 궤적을 디버그 선으로 그릴지 여부 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Fire|Debug")
	bool bDrawDebug = true;
};
