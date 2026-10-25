#include "Character/ShooterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"
#include "GAS/ShooterGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterCharacterBase::AShooterCharacterBase()
{
	// 아직 매 프레임 할 일이 없으므로 Tick 끔 (성능)
	PrimaryActorTick.bCanEverTick = false;

	// 생성자에서 컴포넌트 생성 → 이 캐릭터의 기본 구성품이 된다
	// (이름 "AbilitySystemComponent"/"AttributeSet"은 기존 BP 데이터와 맞추기 위해 그대로 유지)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AttributeSet도 ASC의 Owner(=이 캐릭터)의 서브오브젝트로 만들면
	// ASC가 초기화될 때 자동으로 찾아서 등록한다
	AttributeSet = CreateDefaultSubobject<UShooterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AShooterCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AShooterCharacterBase::IsDead() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(ShooterTags::State_Dead);
}

void AShooterCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// "Health가 바뀌면 OnHealthChanged를 불러줘" 라고 구독 (C#의 event += 핸들러 와 같음)
		// BeginPlay는 한 번만 호출되므로 중복 구독 걱정이 없다
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetHealthAttribute())
			.AddUObject(this, &AShooterCharacterBase::OnHealthChanged);
	}
}

void AShooterCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		// ASC에게 "누가 소유자(Owner)이고, 누가 실제로 움직이는 몸(Avatar)인지" 알려준다.
		// 플레이어든 AI든 캐릭터가 ASC를 직접 들고 있으므로 둘 다 this.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AShooterCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 체력이 0 이하가 됐고, 아직 죽은 상태가 아니면 사망 처리 (중복 사망 방지)
	if (Data.NewValue <= 0.f && !IsDead())
	{
		Die();
	}
}

void AShooterCharacterBase::Die()
{
	// 1) 죽은 상태 태그 부여 → 다른 시스템이 "죽었나?"를 이 태그로 판단
	AbilitySystemComponent->AddLooseGameplayTag(ShooterTags::State_Dead);

	// 2) 더 이상 맞지 않고, 움직이지 않도록
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	UE_LOG(LogTemp, Log, TEXT("[GAS] %s 사망"), *GetName());

	// 3) 연출은 BP에 맡김
	OnDeath();

	// 4) 외부 구독자들에게 "나 죽었다" 알림 (GameMode가 웨이브 진행 판단에 사용)
	//    Broadcast = 등록된 모든 함수 호출 (C#의 OnCharacterDied?.Invoke(this))
	OnCharacterDied.Broadcast(this);
}
