#include "Animation/ShooterAnimInstance.h"
#include "Character/ShooterCharacter.h"
#include "Weapon/ShooterWeapon.h"

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// TryGetPawnOwner: 이 애니메이션을 쓰는 메시의 주인 Pawn (에디터 미리보기 창에서는 없을 수 있음)
	const AShooterCharacter* Character = Cast<AShooterCharacter>(TryGetPawnOwner());
	const AShooterWeapon* Weapon = Character ? Character->GetCurrentWeapon() : nullptr;

	// 무기를 들고 살아 있을 때만 왼손을 총에 붙인다
	if (Weapon && !Character->IsDead())
	{
		LeftHandIKLocation = Weapon->GetLeftHandGripLocation();
		LeftHandIKAlpha = 1.f;
	}
	else
	{
		LeftHandIKAlpha = 0.f;
	}
}
