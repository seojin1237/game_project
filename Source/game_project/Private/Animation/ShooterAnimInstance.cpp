#include "Animation/ShooterAnimInstance.h"
#include "Character/ShooterCharacter.h"
#include "Weapon/ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterGameplayTags.h"

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// TryGetPawnOwner: 이 애니메이션을 쓰는 메시의 주인 Pawn (에디터 미리보기 창에서는 없을 수 있음)
	const AShooterCharacter* Character = Cast<AShooterCharacter>(TryGetPawnOwner());
	const AShooterWeapon* Weapon = Character ? Character->GetCurrentWeapon() : nullptr;

	// 이동 값 계산 (ABP의 Blend Space와 상태 전환이 읽는다)
	if (const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr)
	{
		const FVector Velocity = Movement->Velocity;
		// GetCurrentAcceleration: 입력으로 가하는 가속도 = 플레이어가 가려는 방향
		const FVector Acceleration = Movement->GetCurrentAcceleration();

		// Size2D: Z를 뺀 수평 길이 (Unity의 new Vector2(v.x, v.z).magnitude)
		MoveSpeed = Velocity.Size2D();
		VerticalSpeed = Velocity.Z;
		bShouldMove = MoveSpeed > 0.01f && !Acceleration.IsZero();
		bIsFalling = Movement->IsFalling();

		// 방향을 WASD 입력으로 구하는 이유: 몸이 항상 카메라를 보므로 입력이 곧 몸 기준 방향이다
		// 속도나 가속도로 구하면 마우스를 돌릴 때 값이 흔들려 대각선 애니메이션이 떨린다
		// Atan2(옆, 앞): 앞 = 0도, 오른쪽 = 90도, 왼쪽 = -90도, 뒤 = 180도 (Unity의 Mathf.Atan2와 같음)
		const FVector2D MoveInput = Character->GetLastMoveInput();
		MoveDirection = FMath::RadiansToDegrees(FMath::Atan2(MoveInput.X, MoveInput.Y));

		// 제자리 회전: 몸(캡슐)은 카메라를 따라 바로 돌므로, 서 있을 때만 메시를 그만큼 반대로 돌려 발을 제자리에 둔다
		// FindDeltaAngleDegrees: 두 각도의 차이를 -180 ~ 180도로 계산 (359도 → 1도를 2도 차이로 처리, Unity의 Mathf.DeltaAngle)
		const float ActorYaw = Character->GetActorRotation().Yaw;
		const float YawDelta = bHasLastActorYaw ? FMath::FindDeltaAngleDegrees(LastActorYaw, ActorYaw) : 0.f;
		LastActorYaw = ActorYaw;
		bHasLastActorYaw = true;

		if (MoveSpeed > 1.f || bIsFalling)
		{
			// 움직이면 이동 애니메이션이 몸 기준이어야 하므로 하체를 다시 정면으로 돌린다
			RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.f, DeltaSeconds, RootYawReturnSpeed);
			TurnTimeLeft = 0.f;
			TurnIndex = 0;
		}
		else
		{
			RootYawOffset -= YawDelta;

			if (TurnTimeLeft > 0.f)
			{
				// 도는 중: 발을 옮기는 동안 하체를 정면으로 돌린다
				// FInterpConstantTo: 일정한 속도로 목표까지 이동 (Unity의 Mathf.MoveTowards)
				RootYawOffset = FMath::FInterpConstantTo(RootYawOffset, 0.f, DeltaSeconds, TurnYawSpeed);
				TurnTimeLeft -= DeltaSeconds;
				if (TurnTimeLeft <= 0.f)
				{
					TurnIndex = 0;
				}
			}
			else if (FMath::Abs(RootYawOffset) > TurnStartAngle)
			{
				// 각도가 커졌으니 돌기 시작. 오프셋이 음수 = 몸이 오른쪽으로 돌았다 = 발도 오른쪽으로 옮긴다
				TurnIndex = RootYawOffset < 0.f ? 2 : 1;
				TurnTimeLeft = TurnDuration;
				TurnYawSpeed = FMath::Abs(RootYawOffset) / TurnDuration;
			}

			RootYawOffset = FMath::Clamp(RootYawOffset, -MaxRootYawOffset, MaxRootYawOffset);
		}
		SpineYawCorrection = FRotator(0.f, -RootYawOffset / 3.f, 0.f);
	}

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
	// 조준 중이거나 마지막 사격 후 FireAimHoldTime초 안이면 상체를 겨눈 자세로
	// GetTimeSeconds: 게임 시작 후 흐른 시간(초) (Unity의 Time.time)
	bUpperBodyAim = Character && Weapon && !Character->IsDead()
		&& (Character->IsAiming() || GetWorld()->GetTimeSeconds() - Character->GetLastFireTime() < FireAimHoldTime);

	if (Character)
	{
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(Character->GetBaseAimRotation(), Character->GetActorRotation());
		AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
	}
}
