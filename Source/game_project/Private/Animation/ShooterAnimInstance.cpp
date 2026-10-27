#include "Animation/ShooterAnimInstance.h"
#include "Character/ShooterCharacter.h"
#include "Weapon/ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"

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

	// 위아래 조준 각도 = (카메라가 보는 방향 - 몸이 향한 방향)의 Pitch
	// GetBaseAimRotation: 조종 중인 컨트롤러의 시선 회전 (플레이어면 카메라 방향)
	// NormalizedDeltaRotator: 두 회전의 차이를 -180 ~ 180도로 정리 (Pitch 270도 같은 값이 -90도로 바뀜)
	if (Character)
	{
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(Character->GetBaseAimRotation(), Character->GetActorRotation());
		AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
	}
}
