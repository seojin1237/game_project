#include "UI/ShooterHealthBarWidget.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"
#include "Components/ProgressBar.h"

void UShooterHealthBarWidget::BindToAbilitySystem(UAbilitySystemComponent* InASC)
{
	// 이전 연결이 있으면 먼저 끊는다 (중복 구독 방지)
	Unbind();

	BoundASC = InASC;
	if (!InASC)
	{
		return;
	}

	// Health, MaxHealth 변화 구독 (플레이어 HUD와 같은 방식)
	HealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UShooterHealthBarWidget::HandleHealthChanged);
	MaxHealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UShooterHealthBarWidget::HandleHealthChanged);

	// 구독은 "앞으로의 변화"만 알려주므로 현재 값으로 한 번 초기화
	RefreshHealth();
}

void UShooterHealthBarWidget::NativeDestruct()
{
	Unbind();
	Super::NativeDestruct();
}

void UShooterHealthBarWidget::Unbind()
{
	// ASC가 아직 살아 있을 때만 해제 (Weak 포인터 → 삭제됐으면 nullptr)
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
	}
	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	BoundASC.Reset();
}

void UShooterHealthBarWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshHealth();
}

void UShooterHealthBarWidget::RefreshHealth()
{
	UAbilitySystemComponent* ASC = BoundASC.Get();
	if (!ASC || !HealthBar)
	{
		return;
	}

	const float Health = ASC->GetNumericAttribute(UShooterAttributeSet::GetHealthAttribute());
	const float MaxHealth = ASC->GetNumericAttribute(UShooterAttributeSet::GetMaxHealthAttribute());

	// 0~1 비율로 채움. MaxHealth가 0이면 나눗셈 오류 방지
	HealthBar->SetPercent(MaxHealth > 0.f ? Health / MaxHealth : 0.f);
}
