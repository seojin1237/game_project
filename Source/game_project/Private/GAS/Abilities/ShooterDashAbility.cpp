#include "GAS/Abilities/ShooterDashAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"

void UShooterDashAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 쿨타임 지불. CooldownGameplayEffectClass(GE_Cooldown_Dash)가 여기서 적용된다
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// GetCurrentAcceleration: 지금 입력으로 가려는 월드 방향. 키를 안 누르면 0이다
	// GetSafeNormal2D: Z를 빼고 길이 1로 만든다. 길이가 0이면 0을 돌려줘서 나누기 오류가 없다 (Unity의 Vector3.normalized와 비슷)
	FVector Direction = Movement->GetCurrentAcceleration().GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = Character->GetActorForwardVector();
	}

	// PlaySlotAnimationAsDynamicMontage: 시퀀스를 임시 Montage로 감싸 Slot에 재생한다 (Montage 에셋을 따로 만들 필요 없음)
	// 같은 Slot 그룹의 다른 Montage는 멈추므로, 재장전 중에 대시하면 재장전이 취소된다
	UAnimSequenceBase* DashAnimation = SelectDashAnimation(*Character, Direction);
	UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (DashAnimation && AnimInstance)
	{
		AnimInstance->PlaySlotAnimationAsDynamicMontage(DashAnimation, TEXT("DefaultSlot"), 0.05f, 0.2f, DashAnimationPlayRate);
	}

	// 소리와 이펙트는 GameplayCue에 맡긴다. Location을 넘겨서 대시를 시작한 자리에서 재생되게 한다
	if (DashCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = Character->GetActorLocation();
		GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(DashCueTag, CueParams);
	}

	// ApplyRootMotionConstantForce: 정해진 시간 동안 정해진 속도로 캐릭터를 민다
	// LaunchCharacter(순간 속도 주기)와 달리 마찰의 영향을 받지 않아 이동 거리가 항상 일정하다 (거리 = 속도 × 시간)
	// 끝날 때 ClampVelocity: 대시 속도를 평소 최고 속도로 줄여서, 뚝 멈추지 않고 달리기로 자연스럽게 이어진다
	UAbilityTask_ApplyRootMotionConstantForce* Task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this,
		NAME_None,
		Direction,
		DashDistance / DashDuration,
		DashDuration,
		false,		// bIsAdditive: false = 기존 이동 속도를 무시하고 이 힘만 적용
		nullptr,	// StrengthOverTime: 시간에 따른 세기 커브 (없으면 일정한 속도)
		ERootMotionFinishVelocityMode::ClampVelocity,
		FVector::ZeroVector,	// SetVelocityOnFinish: SetVelocity 모드에서만 쓰는 값
		Movement->GetMaxSpeed(),
		true);		// bEnableGravity: 대시 중 낭떠러지를 넘으면 떨어지게

	Task->OnFinish.AddDynamic(this, &UShooterDashAbility::OnDashFinished);

	// 태스크는 만든 뒤 반드시 시작해야 동작한다
	Task->ReadyForActivation();
}

UAnimSequenceBase* UShooterDashAbility::SelectDashAnimation(const AActor& Avatar, const FVector& Direction) const
{
	// 내적(Dot): 두 방향이 얼마나 같은 쪽을 보는지 (같으면 1, 수직이면 0, 반대면 -1. Unity의 Vector3.Dot)
	const float ForwardAmount = FVector::DotProduct(Direction, Avatar.GetActorForwardVector());
	const float RightAmount = FVector::DotProduct(Direction, Avatar.GetActorRightVector());

	// 대각선 대시는 앞뒤/좌우 중 더 가까운 쪽의 애니메이션을 쓴다
	if (FMath::Abs(ForwardAmount) >= FMath::Abs(RightAmount))
	{
		return ForwardAmount >= 0.f ? DashForwardAnimation : DashBackwardAnimation;
	}
	return RightAmount >= 0.f ? DashRightAnimation : DashLeftAnimation;
}

void UShooterDashAbility::OnDashFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
