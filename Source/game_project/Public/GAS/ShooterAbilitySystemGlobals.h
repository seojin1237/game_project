#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "ShooterAbilitySystemGlobals.generated.h"

/**
 * GAS 전역 설정 클래스. 프로젝트에 하나만 존재한다 (Unity의 싱글톤 매니저와 비슷).
 * 여기서는 "Effect Context를 만들 때 FShooterGameplayEffectContext를 써라"만 바꾼다.
 *
 * 적용 방법: Config/DefaultGame.ini에
 *   [/Script/GameplayAbilities.AbilitySystemGlobals]
 *   AbilitySystemGlobalsClassName="/Script/game_project.ShooterAbilitySystemGlobals"
 */
UCLASS()
class GAME_PROJECT_API UShooterAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	// GAS가 새 Effect Context가 필요할 때 호출 (MakeOutgoingGameplayEffectSpec 등)
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
