#include "GAS/Abilities/ShooterMeleeAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/ShooterCharacterBase.h"
#include "Animation/AnimInstance.h"
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

	// 공격하는 쪽(나) = 이 어빌리티를 가진 ASC의 Avatar
	APawn* Attacker = Cast<APawn>(ActorInfo->AvatarActor.Get());
	UWorld* World = GetWorld();
	if (!Attacker || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2) [선택] 공격 애니메이션 재생. 몽타주가 비어 있으면 건너뜀
	//    AnimInstance = 캐릭터 메시에 붙은 애니메이션 블루프린트 인스턴스 (Unity의 Animator와 비슷)
	//    지금은 "재생만" 하고 판정은 바로 한다. 휘두르는 타이밍에 맞춰 판정하는 건 에셋이 생기면 개선 예정
	if (AttackMontage)
	{
		if (UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance())
		{
			AnimInstance->Montage_Play(AttackMontage);
		}
	}

	// 3) 판정: 내 몸 중심 → 앞쪽 AttackRange까지 구체를 쓸어서(Sweep) 겹치는 Pawn을 모두 찾는다
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

	// 4) 맞은 대상 중 "상대편"에게만 데미지 적용
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

	// 5) 종료 알림 (안 하면 "실행 중"으로 남아 다음 공격이 거부된다)
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
