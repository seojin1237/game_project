#include "GAS/Abilities/ShooterReloadAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/ShooterAttributeSet.h"
#include "Character/ShooterCharacter.h"
#include "Weapon/ShooterWeapon.h"

bool UShooterReloadAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	// 부모 검사(Blocked Tags, 쿨타임, 비용 등)를 먼저 통과해야 한다
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 탄창이 이미 가득이면 재장전할 필요 없음 (괜히 애니메이션만 재생되고 사격이 막히는 것 방지)
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	return ASC && ASC->GetNumericAttribute(UShooterAttributeSet::GetAmmoAttribute())
		< ASC->GetNumericAttribute(UShooterAttributeSet::GetMaxAmmoAttribute());
}

void UShooterReloadAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 비용/쿨타임 지불 (재장전은 둘 다 없지만, 나중에 BP에서 추가해도 동작하도록 규칙대로 호출)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const AShooterCharacter* Character = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get());
	const AShooterWeapon* Weapon = Character ? Character->GetCurrentWeapon() : nullptr;
	UAnimMontage* ReloadMontage = Weapon ? Weapon->GetReloadMontage() : nullptr;

	// 애니메이션이 없으면 바로 채우고 끝
	if (!ReloadMontage)
	{
		RefillAmmo();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// AbilityTask = 어빌리티 안에서 "시간이 걸리는 일"을 맡기는 도우미 (Unity의 코루틴 yield와 비슷)
	// PlayMontageAndWait: Montage를 재생하고, 끝나거나 끊기면 이벤트로 알려준다
	// 어빌리티가 끝나면 Montage도 같이 멈춘다 (bStopWhenAbilityEnds 기본값 true)
	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ReloadMontage);

	// BlendOut = 애니메이션이 자연스럽게 끝나 섞여 나가기 시작하는 순간. 여기서 채우면 끝나기를 기다리는 느낌이 덜하다
	Task->OnBlendOut.AddDynamic(this, &UShooterReloadAbility::OnReloadFinished);
	Task->OnCompleted.AddDynamic(this, &UShooterReloadAbility::OnReloadFinished);
	// Interrupted = 다른 Montage(교체 애니메이션 등)가 덮어씀 / Cancelled = 어빌리티가 취소됨
	Task->OnInterrupted.AddDynamic(this, &UShooterReloadAbility::OnReloadInterrupted);
	Task->OnCancelled.AddDynamic(this, &UShooterReloadAbility::OnReloadInterrupted);

	// 태스크는 만든 뒤 반드시 시작해야 동작한다
	Task->ReadyForActivation();
}

void UShooterReloadAbility::OnReloadFinished()
{
	// BlendOut 다음에 Completed가 또 올 수 있지만, 첫 번째에서 어빌리티를 끝내면 태스크도 정리되어 두 번 채워지지 않는다
	RefillAmmo();
	// Current~: 지금 실행 중인 이 어빌리티의 핸들/정보 (ActivateAbility에서 받았던 값과 같음)
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UShooterReloadAbility::OnReloadInterrupted()
{
	// 마지막 인자 true = "취소되어 끝났음" (탄약은 건드리지 않는다)
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UShooterReloadAbility::RefillAmmo() const
{
	// 예비 탄약은 무한 → 항상 MaxAmmo까지 채운다 (예비 탄약 시스템을 넣으면 여기서 그만큼만 옮기면 된다)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->SetNumericAttributeBase(UShooterAttributeSet::GetAmmoAttribute(),
			ASC->GetNumericAttribute(UShooterAttributeSet::GetMaxAmmoAttribute()));
	}
}
