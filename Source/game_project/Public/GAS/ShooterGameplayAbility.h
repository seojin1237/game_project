#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ShooterGameplayAbility.generated.h"

/**
 * 이 프로젝트의 모든 어빌리티(GA_)가 상속할 베이스 클래스.
 * 공통 기능(쿨타임 표시, 입력 연결 등)이 생기면 여기에 추가한다.
 */
UCLASS()
class GAME_PROJECT_API UShooterGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UShooterGameplayAbility();
};
