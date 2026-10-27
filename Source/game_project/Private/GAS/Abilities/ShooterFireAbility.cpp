#include "GAS/Abilities/ShooterFireAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Game/ShooterCollision.h"
#include "Character/ShooterCharacter.h"
#include "Weapon/ShooterWeapon.h"

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

	// 2) 들고 있는 무기의 데이터 사용. 무기가 없으면 이 어빌리티(GA_Fire)에 지정한 값으로 대체
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Avatar);
	const AShooterWeapon* Weapon = ShooterCharacter ? ShooterCharacter->GetCurrentWeapon() : nullptr;
	const TSubclassOf<UGameplayEffect> UsedDamageEffect = (Weapon && Weapon->GetDamageEffect()) ? Weapon->GetDamageEffect() : DamageEffect;
	const float UsedRange = Weapon ? Weapon->GetRange() : Range;

	// 사격 애니메이션(반동) 재생. 연사 중이면 처음부터 다시 재생되어 매 발마다 반동이 보인다
	// PlayAnimMontage: ACharacter 기본 함수. 메시의 AnimInstance에 Montage 재생을 요청 (Unity의 Animator.Play와 비슷)
	if (ShooterCharacter && Weapon && Weapon->GetFireMontage())
	{
		ShooterCharacter->PlayAnimMontage(Weapon->GetFireMontage());
	}

	// 총구 화염. 어떤 이펙트를 쓸지는 무기가 알고 있으므로 무기에게 맡긴다
	if (Weapon)
	{
		Weapon->PlayMuzzleFlash();
	}

	// 쏜 사람과 그 무기는 트레이스에서 무시
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ShooterFire), false, Avatar);
	if (Weapon)
	{
		Params.AddIgnoredActor(Weapon);
	}

	// 3) 조준점 찾기: 카메라(화면 중앙 = 크로스헤어)에서 앞으로 트레이스
	FVector ViewLocation = Avatar->GetActorLocation();
	FRotator ViewRotation = Avatar->GetActorRotation();
	if (const APawn* Pawn = Cast<APawn>(Avatar))
	{
		if (AController* Controller = Pawn->GetController())
		{
			// out 매개변수: 함수가 ViewLocation과 ViewRotation에 값을 채워 넣는다 (C#의 out 키워드와 같음)
			Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
		}
	}
	const FVector ViewDir = ViewRotation.Vector();

	// 카메라는 캐릭터 뒤에 있으므로, 트레이스 시작을 "캐릭터 위치까지" 앞으로 당긴다
	// → 캐릭터와 카메라 사이에 있는 물체(뒤에 있는 적 등)를 조준점으로 잡지 않게
	// FVector::DotProduct: 두 벡터의 내적 = 카메라→캐릭터 거리 중 시선 방향 성분 (Unity의 Vector3.Dot)
	const float DistToAvatar = FMath::Max(0.f, FVector::DotProduct(Avatar->GetActorLocation() - ViewLocation, ViewDir));
	const FVector AimStart = ViewLocation + ViewDir * DistToAvatar;
	const FVector AimEnd = AimStart + ViewDir * UsedRange;

	FHitResult AimHit;
	const FVector AimPoint = World->LineTraceSingleByChannel(AimHit, AimStart, AimEnd, ECC_Weapon, Params)
		? AimHit.ImpactPoint : AimEnd;

	// 4) 실제 총알: 총구에서 조준점을 향해 트레이스 (무기가 없으면 예전처럼 카메라 기준)
	//    채널은 무기 전용 ECC_Weapon. ECC_Visibility는 캐릭터 캡슐(Pawn 프리셋)이 무시해서 적이 안 맞았다
	const FVector Start = Weapon ? Weapon->GetMuzzleLocation() : AimStart;
	// 조준점에서 딱 멈추면 표면에 살짝 못 미칠 수 있어 10cm 더 연장
	const FVector End = AimPoint + (AimPoint - Start).GetSafeNormal() * 10.f;

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Weapon, Params);

	if (bDrawDebug)
	{
		// 맞으면 빨간 선 + 맞은 지점에 구, 빗나가면 초록 선 (1초 동안 표시)
		DrawDebugLine(World, Start, bHit ? Hit.ImpactPoint : End, bHit ? FColor::Red : FColor::Green, false, 1.f, 0, 1.f);
		if (bHit)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 10.f, 8, FColor::Red, false, 1.f);
		}
	}

	// 5) 맞은 대상이 ASC를 가지고 있으면 데미지 Effect 적용
	if (bHit && UsedDamageEffect)
	{
		// IAbilitySystemInterface를 구현한 액터면 ASC를 돌려준다 (없으면 nullptr)
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Hit.GetActor()))
		{
			// 어빌리티용 Spec 생성 함수: Context(쏜 사람 정보)가 자동으로 채워진다
			FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(UsedDamageEffect, GetAbilityLevel());
			if (Spec.IsValid())
			{
				// 어디를 맞았는지도 기록 → 나중에 피격 이펙트(GameplayCue) 위치로 사용
				Spec.Data->GetContext().AddHitResult(Hit);

				// 내 ASC가 "가해자"가 되어 상대 ASC에 적용
				GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
	}

	// 6) 반드시 종료 알림. 안 하면 "실행 중"으로 남아 다음 클릭이 거부된다
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
