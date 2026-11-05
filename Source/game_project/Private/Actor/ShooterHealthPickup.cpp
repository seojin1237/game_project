#include "Actor/ShooterHealthPickup.h"
#include "AbilitySystemComponent.h"
#include "Character/ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GAS/ShooterAttributeSet.h"

AShooterHealthPickup::AShooterHealthPickup()
{
	// 떠다니는 움직임을 Tick에서 처리한다
	PrimaryActorTick.bCanEverTick = true;

	// 안 먹고 내버려 두면 이 시간(초) 뒤 자동으로 사라진다 (Unity의 Destroy(gameObject, 15f)). 실제 튜닝은 BP에서
	InitialLifeSpan = 15.f;

	// Pawn만 겹침으로 감지한다. 총알(ECC_Weapon)은 통과하고, 플레이어의 이동도 막지 않는다
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(60.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(PickupSphere);

	// 충돌은 PickupSphere가 맡으므로 메시는 보이기만 한다
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(PickupSphere);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 생성자에서 연결하는 이유: 플레이어 바로 위에 스폰돼도, 스폰 순간의 겹침 이벤트를 놓치지 않게
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AShooterHealthPickup::OnPickupOverlap);
}

void AShooterHealthPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 충돌 범위(PickupSphere)는 그대로 두고 보이는 메시만 움직인다
	// Sin: -1 ~ 1을 반복하는 값 (Unity의 Mathf.Sin). GetGameTimeSinceCreation: 이 액터가 생긴 뒤 흐른 시간(초)
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, FMath::Sin(GetGameTimeSinceCreation() * BobSpeed) * BobHeight));
}

void AShooterHealthPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 적도 Pawn이라 여기로 들어오므로 플레이어만 통과시킨다
	const AShooterCharacter* Player = Cast<AShooterCharacter>(OtherActor);
	UAbilitySystemComponent* ASC = Player ? Player->GetAbilitySystemComponent() : nullptr;
	if (!ASC || !HealEffect || Player->IsDead())
	{
		return;
	}

	// 체력이 가득이면 먹지 않고 남겨 둔다 (아이템을 낭비하지 않게)
	// shortcut: 올라선 채로 맞으면 다시 닿기 전까지 안 먹힌다. 불편하면 Timer로 겹침을 다시 검사할 것
	if (ASC->GetNumericAttribute(UShooterAttributeSet::GetHealthAttribute()) >= ASC->GetNumericAttribute(UShooterAttributeSet::GetMaxHealthAttribute()))
	{
		return;
	}

	// GE 적용. 최대 체력을 넘지 않게 자르는 일은 AttributeSet이 처리한다
	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(HealEffect, 1.f, ASC->MakeEffectContext());
	if (Spec.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	// 소리와 이펙트는 GameplayCue에 맡긴다. 액터가 곧 사라져도 Cue는 이 위치에서 끝까지 재생된다
	if (PickupCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		ASC->ExecuteGameplayCue(PickupCueTag, CueParams);
	}

	Destroy();
}
