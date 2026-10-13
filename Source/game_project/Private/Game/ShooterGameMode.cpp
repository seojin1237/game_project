#include "Game/ShooterGameMode.h"
#include "Character/ShooterCharacter.h"

AShooterGameMode::AShooterGameMode()
{
	// C++ 기본값. 실제로는 BP_ShooterGameMode에서 BP_ShooterCharacter로 바꿔서 사용
	DefaultPawnClass = AShooterCharacter::StaticClass();
}
