#include "Character/ShooterCharacter.h"
#include "AbilitySystemComponent.h"

AShooterCharacter::AShooterCharacter()
{
	// 아직 매 프레임 할 일이 없으므로 Tick 끔 (성능)
	PrimaryActorTick.bCanEverTick = false;

	// 생성자에서 컴포넌트 생성 → 이 캐릭터의 기본 구성품이 된다
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* AShooterCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		// ASC에게 "누가 소유자(Owner)이고, 누가 실제로 움직이는 몸(Avatar)인지" 알려준다.
		// 이 캐릭터는 ASC를 직접 들고 있으므로 둘 다 this.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
