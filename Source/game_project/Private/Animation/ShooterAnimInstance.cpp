#include "Animation/ShooterAnimInstance.h"
#include "Character/ShooterCharacter.h"
#include "Weapon/ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterGameplayTags.h"

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// TryGetPawnOwner: 이 애니메이션을 쓰는 메시의 주인 Pawn (에디터 미리보기 창에서는 없을 수 있음)
	const AShooterCharacter* Character = Cast<AShooterCharacter>(TryGetPawnOwner());
	const AShooterWeapon* Weapon = Character ? Character->GetCurrentWeapon() : nullptr;

	// 무기를 들고 살아 있을 때만 왼손을 총에 붙인다
	// 단, 재장전 중에는 왼손이 탄창을 갈아 끼워야 하므로 IK를 풀어서 애니메이션의 손 움직임을 그대로 쓴다
	const UAbilitySystemComponent* ASC = Character ? Character->GetAbilitySystemComponent() : nullptr;
	const bool bReloading = ASC && ASC->HasMatchingGameplayTag(ShooterTags::State_Reloading);
	float TargetAlpha = 0.f;
	if (Weapon && !Character->IsDead())
	{
		LeftHandIKLocation = Weapon->GetLeftHandGripLocation();
		TargetAlpha = bReloading ? 0.f : 1.f;
	}

	// 0 ↔ 1을 한 프레임에 바꾸면 손이 순간이동하므로 부드럽게 따라가게 한다
	// FInterpTo(현재, 목표, 프레임 시간, 속도): 매 프레임 목표 쪽으로 조금씩 이동 (Unity의 Mathf.Lerp(a, b, t * speed)와 비슷)
	LeftHandIKAlpha = FMath::FInterpTo(LeftHandIKAlpha, TargetAlpha, DeltaSeconds, 12.f);

	// 위아래 조준 각도 = (카메라가 보는 방향 - 몸이 향한 방향)의 Pitch
	// GetBaseAimRotation: 조종 중인 컨트롤러의 시선 회전 (플레이어면 카메라 방향)
	// NormalizedDeltaRotator: 두 회전의 차이를 -180 ~ 180도로 정리 (Pitch 270도 같은 값이 -90도로 바뀜)
	if (Character)
	{
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(Character->GetBaseAimRotation(), Character->GetActorRotation());
		AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
	}
}
