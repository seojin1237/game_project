#include "GAS/Cues/ShooterGCN_DamageNumber.h"
#include "Actor/ShooterDamageNumberActor.h"
#include "GameFramework/Pawn.h"

bool UShooterGCN_DamageNumber::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !DamageNumberClass)
	{
		return false;
	}

	// 플레이어가 맞은 경우는 기본적으로 표시 안 함 (HUD 체력 바로 충분)
	// IsPlayerControlled: 사람(PlayerController)이 조종하는 Pawn인지
	if (!bShowWhenPlayerIsHit)
	{
		if (const APawn* TargetPawn = Cast<APawn>(MyTarget))
		{
			if (TargetPawn->IsPlayerControlled())
			{
				return false;
			}
		}
	}

	// RawMagnitude: 이번 GE가 Health를 바꾼 양. 데미지면 음수(-20)이므로 부호를 뒤집는다
	const float Damage = -Parameters.RawMagnitude;
	if (Damage <= 0.f)
	{
		return false; // 회복 등 데미지가 아닌 경우는 숫자를 띄우지 않음
	}

	// 띄울 위치: 맞은 지점(사격/근접 어빌리티가 AddHitResult로 넣어둔 값)이 있으면 거기, 없으면 대상 머리 위
	FVector SpawnLocation = MyTarget->GetActorLocation() + FVector(0.f, 0.f, FallbackHeight);
	if (const FHitResult* Hit = Parameters.EffectContext.GetHitResult())
	{
		SpawnLocation = Hit->ImpactPoint;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World)
	{
		return false;
	}

	// 숫자 액터 스폰 → 데미지 값 전달. 이후 움직임/삭제는 액터가 알아서 한다
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AShooterDamageNumberActor* Number = World->SpawnActor<AShooterDamageNumberActor>(
		DamageNumberClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Number->SetDamage(Damage);
	}

	return true;
}
