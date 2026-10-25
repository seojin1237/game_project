#include "Game/ShooterGameMode.h"
#include "Character/ShooterCharacter.h"
#include "Game/ShooterPlayerController.h"

AShooterGameMode::AShooterGameMode()
{
	// C++ 기본값. 실제로는 BP_ShooterGameMode에서 BP_ShooterCharacter로 바꿔서 사용
	DefaultPawnClass = AShooterCharacter::StaticClass();

	// 플레이어가 접속하면 만들어질 컨트롤러 클래스. 실제로는 BP에서 BP_ShooterPlayerController로 바꿔서 사용
	PlayerControllerClass = AShooterPlayerController::StaticClass();
}
