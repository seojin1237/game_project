#include "Actor/ShooterDamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "UI/ShooterDamageNumberWidget.h"

AShooterDamageNumberActor::AShooterDamageNumberActor()
{
	// 매 프레임 위치/투명도를 바꿔야 하므로 Tick 켬
	PrimaryActorTick.bCanEverTick = true;

	NumberWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NumberWidget"));
	SetRootComponent(NumberWidget);

	// Screen: 3D 공간에 있지만 항상 화면을 정면으로 보고, 거리와 상관없이 같은 크기로 그린다
	// (World로 하면 3D 판자처럼 보이고 멀수록 작아짐)
	NumberWidget->SetWidgetSpace(EWidgetSpace::Screen);
	// 위젯의 원래 크기(글자 크기)대로 그린다
	NumberWidget->SetDrawAtDesiredSize(true);
	// 숫자는 충돌이 필요 없다 (총알이 숫자에 막히면 안 되니까)
	NumberWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShooterDamageNumberActor::BeginPlay()
{
	Super::BeginPlay();

	// Lifetime초 뒤 자동 삭제 (Unity의 Destroy(gameObject, lifetime))
	SetLifeSpan(Lifetime);

	// 좌우로 살짝 랜덤하게 흩뜨림. VRand(): 길이 1짜리 랜덤 방향 벡터
	FVector Offset = FMath::VRand() * RandomOffset;
	Offset.Z = FMath::Abs(Offset.Z); // 아래로는 내려가지 않게
	AddActorWorldOffset(Offset);
}

void AShooterDamageNumberActor::SetDamage(float Damage)
{
	// GetUserWidgetObject(): WidgetComponent가 실제로 만든 위젯 인스턴스
	if (UShooterDamageNumberWidget* Widget = Cast<UShooterDamageNumberWidget>(NumberWidget->GetUserWidgetObject()))
	{
		Widget->SetDamage(Damage);
	}
}

void AShooterDamageNumberActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;

	// 위로 이동 (프레임 시간을 곱해서 프레임 속도와 무관하게 일정한 속도로)
	AddActorWorldOffset(FVector(0.f, 0.f, RiseSpeed * DeltaSeconds));

	// 수명의 절반이 지나면 점점 투명해짐 (1 → 0)
	if (UUserWidget* Widget = NumberWidget->GetUserWidgetObject())
	{
		const float FadeStart = Lifetime * 0.5f;
		const float Alpha = (Age <= FadeStart) ? 1.f : 1.f - (Age - FadeStart) / (Lifetime - FadeStart);
		Widget->SetRenderOpacity(FMath::Clamp(Alpha, 0.f, 1.f));
	}
}
