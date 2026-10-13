#include "Character/ShooterCharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"

AShooterCharacter::AShooterCharacter()
{
	// 아직 매 프레임 할 일이 없으므로 Tick 끔 (성능)
	PrimaryActorTick.bCanEverTick = false;

	// 생성자에서 컴포넌트 생성 → 이 캐릭터의 기본 구성품이 된다
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AttributeSet도 ASC의 Owner(=이 캐릭터)의 서브오브젝트로 만들면
	// ASC가 초기화될 때 자동으로 찾아서 등록한다
	AttributeSet = CreateDefaultSubobject<UShooterAttributeSet>(TEXT("AttributeSet"));
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

void AShooterCharacter::DebugDamage()
{
	if (!AbilitySystemComponent || !DebugDamageEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAS] DebugDamageEffect가 비어 있습니다. BP에서 GE_Damage를 지정하세요."));
		return;
	}

	// 1) Context: "누가, 무엇으로" 이 효과를 일으켰는지 담는 정보
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	// 2) Spec: GE 클래스(설계도) + 레벨 + Context를 묶은 "적용 준비가 끝난 실제 효과"
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DebugDamageEffect, 1.f, Context);

	// 3) 적용: 지금은 테스트라 자기 자신에게
	if (Spec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}
