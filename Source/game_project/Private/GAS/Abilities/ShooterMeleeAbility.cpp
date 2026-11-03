#include "GAS/Abilities/ShooterMeleeAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/ShooterCharacterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "DrawDebugHelpers.h"

UShooterMeleeAbility::UShooterMeleeAbility()
{
}

void UShooterMeleeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 1) Commit: 쿨타임 GE를 실제로 적용 (BP에서 Cooldown GE를 지정해두면 자동으로 공격 간격 제한)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2) 몽타주가 없으면 바로 판정하고 끝
	if (!AttackMontage)
	{
		PerformAttack();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 3) 공격 몽타주 재생. 끝나거나 끊기면 어빌리티 종료 (재장전 어빌리티와 같은 방식)
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
	MontageTask->OnBlendOut.AddDynamic(this, &UShooterMeleeAbility::OnAttackFinished);
	MontageTask->OnCompleted.AddDynamic(this, &UShooterMeleeAbility::OnAttackFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UShooterMeleeAbility::OnAttackFinished);
	MontageTask->OnCancelled.AddDynamic(this, &UShooterMeleeAbility::OnAttackFinished);
	MontageTask->ReadyForActivation();

	// 4) HitDelay초 뒤 판정. 휘두르는 동안 플레이어가 물러나면 피할 수 있다
	//    WaitDelay: 어빌리티 안에서 쓰는 대기 태스크 (Unity 코루틴의 yield return new WaitForSeconds)
	//    어빌리티가 먼저 끝나거나 취소되면(적 사망 등) 태스크도 같이 사라져 판정이 나가지 않는다
	UAbilityTask_WaitDelay* HitTask = UAbilityTask_WaitDelay::WaitDelay(this, HitDelay);
	HitTask->OnFinish.AddDynamic(this, &UShooterMeleeAbility::OnHitTime);
	HitTask->ReadyForActivation();
}

void UShooterMeleeAbility::OnHitTime()
{
	PerformAttack();
}

void UShooterMeleeAbility::OnAttackFinished()
{
	// BlendOut 다음에 Completed가 또 올 수 있지만, 첫 번째에서 어빌리티를 끝내면 태스크도 정리된다
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UShooterMeleeAbility::PerformAttack()
{
	// 공격하는 쪽(나) = 이 어빌리티를 가진 ASC의 Avatar
	APawn* Attacker = Cast<APawn>(GetAvatarActorFromActorInfo());
	UWorld* World = GetWorld();
	if (!Attacker || !World)
	{
		return;
	}

	// 판정: 내 몸 중심 → 앞쪽 AttackRange까지 구체를 쓸어서(Sweep) 겹치는 Pawn을 모두 찾는다
	//    Sweep = 도형을 이동시키며 충돌 검사 (Unity의 Physics.SphereCastAll과 같은 개념)
	const FVector Start = Attacker->GetActorLocation();
	const FVector End = Start + Attacker->GetActorForwardVector() * AttackRange;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ShooterMelee), false, Attacker); // 나 자신은 무시
	TArray<FHitResult> Hits;
	World->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params);

	if (bDrawDebug)
	{
		// 휘두른 범위를 1초간 주황색 구체로 표시 (시작/끝 지점)
		DrawDebugSphere(World, Start, AttackRadius, 12, FColor::Orange, false, 1.f);
		DrawDebugSphere(World, End, AttackRadius, 12, FColor::Orange, false, 1.f);
	}

	// 맞은 대상 중 "상대편"에게만 데미지 적용
	//    TSet: 중복 없는 집합 (C#의 HashSet). 한 캐릭터가 여러 번 검출돼도 데미지는 한 번만
	TSet<AActor*> AlreadyHit;
	for (const FHitResult& Hit : Hits)
	{
		AShooterCharacterBase* Target = Cast<AShooterCharacterBase>(Hit.GetActor());
		if (!Target || Target->IsDead() || AlreadyHit.Contains(Target))
		{
			continue;
		}

		// 같은 편(둘 다 플레이어 조종이거나 둘 다 AI 조종)이면 무시 → 적끼리 서로 안 때림
		if (Target->IsPlayerControlled() == Attacker->IsPlayerControlled())
		{
			continue;
		}
		AlreadyHit.Add(Target);

		if (!DamageEffect)
		{
			continue;
		}

		// 대상의 ASC를 찾아서 데미지 Effect 적용 (사격 어빌리티와 같은 방식)
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
		{
			FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
			if (Spec.IsValid())
			{
				// 맞은 위치 기록 → GE_Damage에 연결된 피격 Cue(GC_Hit_Impact)가 이 위치에 이펙트 재생
				Spec.Data->GetContext().AddHitResult(Hit);
				GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
	}
}
