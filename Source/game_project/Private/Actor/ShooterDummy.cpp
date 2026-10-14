#include "Actor/ShooterDummy.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"
#include "GAS/ShooterGameplayTags.h"

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

		// "Health가 바뀌면 OnHealthChanged를 불러줘" 라고 구독 (C#의 event += 핸들러 와 같음)
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetHealthAttribute())
			.AddUObject(this, &AShooterDummy::OnHealthChanged);
	}
}

void AShooterDummy::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 체력이 0 이하가 됐고, 아직 죽은 상태가 아니면 사망 처리 (중복 사망 방지)
	if (Data.NewValue <= 0.f && !AbilitySystemComponent->HasMatchingGameplayTag(ShooterTags::State_Dead))
	{
		Die();
	}
}

void AShooterDummy::Die()
{
	// 1) 죽은 상태 태그 부여. Loose 태그 = GE 없이 코드로 직접 붙이는 태그
	//    → 다른 시스템(AI, 어빌리티 발동 조건 등)이 "죽었나?"를 이 태그로 판단한다
	AbilitySystemComponent->AddLooseGameplayTag(ShooterTags::State_Dead);

	// 2) 더 이상 총알에 맞지 않도록 충돌 끄기
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UE_LOG(LogTemp, Log, TEXT("[GAS] %s 사망"), *GetName());

	// 3) 연출은 BP에 맡김 (BP에서 OnDeath 이벤트를 구현하지 않았으면 아무 일도 안 일어남)
	OnDeath();

	// 4) DeathLifeSpan초 뒤 자동 삭제 (Unity의 Destroy(gameObject, delay)와 같음)
	if (DeathLifeSpan > 0.f)
	{
		SetLifeSpan(DeathLifeSpan);
	}
}
