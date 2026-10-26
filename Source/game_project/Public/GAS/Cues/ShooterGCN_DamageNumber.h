#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "ShooterGCN_DamageNumber.generated.h"

class AShooterDamageNumberActor;

/**
 * 데미지 숫자를 띄우는 GameplayCue (GC_Hit_DamageNumber의 C++ 부모).
 *
 * GameplayCue 복습: GE에 Cue 태그를 달아두면, GE가 적용될 때 GAS가 그 태그의 Cue를 실행해준다.
 * → 데미지 로직(GE)은 숫자 연출을 전혀 몰라도 되고, 숫자를 바꾸고 싶으면 이 Cue만 고치면 된다.
 *
 * GameplayCueNotify_Static: 인스턴스를 만들지 않고 클래스 기본 객체(CDO)로 실행되는 가벼운 Cue.
 * "한 번 펑" 하는 연출용 (지난번 피격 이펙트의 Burst도 이 계열). 그래서 멤버 변수에 상태를 저장하면 안 된다.
 *
 * 데미지 값은 Cue 파라미터의 RawMagnitude로 받는다.
 * GE_Damage의 Gameplay Cues 항목에서 Magnitude Attribute를 Health로 지정하면,
 * "이번 GE가 Health를 얼마나 바꿨는지"(-20 등)가 RawMagnitude로 들어온다.
 */
UCLASS()
class GAME_PROJECT_API UShooterGCN_DamageNumber : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	// Instant GE(데미지)가 적용될 때 GAS가 호출 (Execute = 한 번 실행되는 Cue)
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	// 스폰할 숫자 액터 (BP에서 BP_DamageNumber 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "DamageNumber" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	TSubclassOf<AShooterDamageNumberActor> DamageNumberClass;

	// 플레이어가 맞았을 때도 숫자를 띄울지 (기본: 적이 맞았을 때만) (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	bool bShowWhenPlayerIsHit = false;

	// 맞은 위치를 모를 때(HitResult가 없을 때) 대상 중심에서 얼마나 위에 띄울지 (cm) (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	float FallbackHeight = 80.f;
};
