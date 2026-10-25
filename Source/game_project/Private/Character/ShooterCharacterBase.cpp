#include "Character/ShooterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"
#include "GAS/ShooterGameplayTags.h"
#include "GAS/ShooterUpgradeData.h"
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

		// 이동 속도 업그레이드/슬로우 디버프 등이 MoveSpeed를 바꾸면 실제 이동 속도에도 반영
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetMoveSpeedAttribute())
			.AddUObject(this, &AShooterCharacterBase::OnMoveSpeedChanged);
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

		// ASC 준비가 끝났으니 초기 스탯 적용
		InitializeAttributes();
	}
}

void AShooterCharacterBase::InitializeAttributes()
{
	// 이미 적용했거나, BP에서 초기 스탯 GE를 지정하지 않았으면 AttributeSet 생성자의 기본값을 그대로 사용
	if (bAttributesInitialized || !AbilitySystemComponent || !DefaultAttributesEffect)
	{
		return;
	}

	// 자기 자신에게 GE 적용 (DebugDamage와 같은 Context → Spec → Apply 흐름)
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.f, Context);
	if (Spec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		bAttributesInitialized = true;
	}

	// 주의: 빙의(PossessedBy)는 BeginPlay보다 먼저 일어나서, BeginPlay에서 구독하는 OnMoveSpeedChanged가
	// 이 초기화 변화를 못 받는다. 그래서 여기서 한 번 직접 반영해준다
	if (AttributeSet)
	{
		GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetMoveSpeed();
	}
}

bool AShooterCharacterBase::ApplyUpgrade(const UShooterUpgradeData* Upgrade)
{
	// 죽었거나, 데이터가 비었거나, GE가 지정 안 된 업그레이드는 무시
	if (IsDead() || !AbilitySystemComponent || !Upgrade || !Upgrade->Effect)
	{
		return false;
	}

	// 초기 스탯과 같은 방식으로 자기 자신에게 GE 적용.
	// 업그레이드 GE는 Infinite라서 적용된 뒤 계속 남아 있고, 같은 업그레이드를 또 고르면 또 하나가 쌓인다
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(Upgrade->Effect, 1.f, Context);
	if (!Spec.IsValid())
	{
		return false;
	}
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	// ToString(): FText → FString 변환 (로그 출력용)
	UE_LOG(LogTemp, Log, TEXT("[Upgrade] %s 적용: %s"), *GetName(), *Upgrade->DisplayName.ToString());
	return true;
}

void AShooterCharacterBase::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	// GAS의 MoveSpeed(스탯) → 엔진의 CharacterMovement(실제 이동) 로 연결하는 다리 역할
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
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
