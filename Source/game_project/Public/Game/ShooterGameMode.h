#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

/**
 * 게임 규칙 담당. 나중에 웨이브 진행 로직이 여기에 들어갈 예정.
 * 지금은 기본 스폰 캐릭터만 지정한다.
 */
UCLASS()
class GAME_PROJECT_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShooterGameMode();
};
