#include "Character/ShooterEnemy.h"
#include "AI/ShooterAIController.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/ShooterHealthBarWidget.h"

AShooterEnemy::AShooterEnemy()
{
	// 레벨에 배치했을 때뿐 아니라 나중에 웨이브로 스폰될 때도 AI 컨트롤러가 자동으로 빙의하게
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 자동 빙의할 때 어떤 AI 컨트롤러를 만들지 지정 (기본값은 아무것도 안 하는 AAIController)
	// StaticClass(): 클래스 자체를 가리키는 값 (C#의 typeof(AShooterAIController)와 비슷)
	AIControllerClass = AShooterAIController::StaticClass();

	// 적은 이동하는 방향으로 몸을 돌린다 (플레이어와 달리 카메라가 없으므로)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 플레이어(기본 600)보다 느리게 → 도망치면서 쏠 수 있게. 실제 튜닝은 BP_Enemy에서
	GetCharacterMovement()->MaxWalkSpeed = 350.f;

	// ===== 머리 위 체력 바 =====
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	// 캡슐 중심에서 위로 (캡슐 반높이 88 + 여유). 실제 위치 조정은 BP에서
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	// Screen: 항상 화면을 정면으로 보고, 멀어져도 같은 크기로 그린다 (3D 판자처럼 기울지 않음)
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	// 화면에 그릴 크기 (픽셀). 실제 크기 조정은 BP에서
	HealthBarWidget->SetDrawSize(FVector2D(80.f, 8.f));
	// 체력 바는 충돌이 필요 없다 (총알이 체력 바에 막히면 안 되니까)
	HealthBarWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShooterEnemy::BeginPlay()
{
	// Super::BeginPlay() 안에서 컴포넌트들의 BeginPlay도 실행되며, 이때 WidgetComponent가 위젯 인스턴스를 만든다
	Super::BeginPlay();

	// 만들어진 체력 바 위젯을 이 적의 ASC에 연결 → 체력이 바뀌면 바가 자동 갱신
	// (초기 스탯 GE는 빙의(PossessedBy) 때 이미 적용됐으므로, 연결 즉시 올바른 값이 표시된다)
	if (UShooterHealthBarWidget* Bar = Cast<UShooterHealthBarWidget>(HealthBarWidget->GetUserWidgetObject()))
	{
		Bar->BindToAbilitySystem(AbilitySystemComponent);
	}
}

void AShooterEnemy::PossessedBy(AController* NewController)
{
	// 부모: ASC 초기화(InitAbilityActorInfo)
	Super::PossessedBy(NewController);

	// 공격 어빌리티 등록(Grant). 플레이어가 GA_Fire를 등록하는 것과 같은 방식
	// FindAbilitySpecFromClass로 이미 등록돼 있는지 확인 → 재빙의 시 중복 등록 방지
	if (AbilitySystemComponent && AttackAbility && !AbilitySystemComponent->FindAbilitySpecFromClass(AttackAbility))
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbility, 1));
	}
}

bool AShooterEnemy::TryAttack(const AActor* Target)
{
	if (IsDead() || !AbilitySystemComponent || !AttackAbility)
	{
		return false;
	}

	// 공격 전에 대상 쪽으로 몸을 돌린다.
	// 이동 방향으로 회전하는 설정이라, 멈춰 있을 때는 플레이어를 정확히 안 보고 있을 수 있기 때문
	// (근접 판정이 "내 앞쪽"으로 나가므로 방향이 중요)
	if (Target)
	{
		FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f; // 위아래 기울기는 무시하고 수평으로만 회전
		if (!ToTarget.IsNearlyZero())
		{
			SetActorRotation(ToTarget.Rotation());
		}
	}

	// 등록된 공격 어빌리티 실행 시도. 쿨타임 중이면 GAS가 false를 돌려준다
	return AbilitySystemComponent->TryActivateAbilityByClass(AttackAbility);
}

void AShooterEnemy::Die()
{
	// 부모 클래스의 공통 사망 처리 (태그, 충돌/이동 끄기, OnDeath 연출) 먼저 실행
	Super::Die();

	// 죽은 적의 체력 바는 바로 숨김 (시체가 사라질 때까지 빈 바가 떠 있지 않게)
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}

	// 적만의 추가 처리: DeathLifeSpan초 뒤 자동 삭제 (Unity의 Destroy(gameObject, delay))
	if (DeathLifeSpan > 0.f)
	{
		SetLifeSpan(DeathLifeSpan);
	}
}
