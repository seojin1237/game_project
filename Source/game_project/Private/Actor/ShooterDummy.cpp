#include "Actor/ShooterDummy.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"

AShooterDummy::AShooterDummy()
{
	PrimaryActorTick.bCanEverTick = false;

	// 메시를 루트로 → 액터 위치 = 메시 위치
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UShooterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AShooterDummy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AShooterDummy::BeginPlay()
{
	Super::BeginPlay();

	// 컨트롤러가 없는 액터라 PossessedBy가 호출되지 않는다 → BeginPlay에서 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
