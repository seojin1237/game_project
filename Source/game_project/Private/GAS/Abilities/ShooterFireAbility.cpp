#include "GAS/Abilities/ShooterFireAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"

UShooterFireAbility::UShooterFireAbility()
{
}

void UShooterFireAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 1) Commit: 비용(탄약/마나)과 쿨타임을 실제로 "지불". 지금은 둘 다 없어서 항상 성공
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	UWorld* World = GetWorld();
	if (!Avatar || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2) 시작점/방향: 플레이어가 보는 시점(카메라) 기준 → 크로스헤어가 있는 화면 중앙으로 쏜다
	FVector Start = Avatar->GetActorLocation();
	FRotator AimRotation = Avatar->GetActorRotation();
	if (const APawn* Pawn = Cast<APawn>(Avatar))
	{
		if (AController* Controller = Pawn->GetController())
		{
			// out 매개변수: 함수가 Start와 AimRotation에 값을 채워 넣는다 (C#의 out 키워드와 같음)
			Controller->GetPlayerViewPoint(Start, AimRotation);
		}
	}
	const FVector End = Start + AimRotation.Vector() * Range;

	// 3) 라인 트레이스 (Unity의 Physics.Raycast). 쏜 사람 자신은 무시
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ShooterFire), false, Avatar);
	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bDrawDebug)
	{
		// 맞으면 빨간 선 + 맞은 지점에 구, 빗나가면 초록 선 (1초 동안 표시)
		DrawDebugLine(World, Start, bHit ? Hit.ImpactPoint : End, bHit ? FColor::Red : FColor::Green, false, 1.f, 0, 1.f);
		if (bHit)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 10.f, 8, FColor::Red, false, 1.f);
		}
	}

	// 4) 맞은 대상이 ASC를 가지고 있으면 데미지 Effect 적용
	if (bHit && DamageEffect)
	{
		// IAbilitySystemInterface를 구현한 액터면 ASC를 돌려준다 (없으면 nullptr)
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Hit.GetActor()))
		{
			// 어빌리티용 Spec 생성 함수: Context(쏜 사람 정보)가 자동으로 채워진다
			FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
			if (Spec.IsValid())
			{
				// 어디를 맞았는지도 기록 → 나중에 피격 이펙트(GameplayCue) 위치로 사용
				Spec.Data->GetContext().AddHitResult(Hit);

				// 내 ASC가 "가해자"가 되어 상대 ASC에 적용
				GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
	}

	// 5) 반드시 종료 알림. 안 하면 "실행 중"으로 남아 다음 클릭이 거부된다
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
