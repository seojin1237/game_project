#include "UI/ShooterHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"
#include "GAS/ShooterGameplayTags.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Game/ShooterGameMode.h"

void UShooterHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// GetAuthGameMode<T>(): 현재 월드의 GameMode를 T 타입으로 얻는다 (싱글플레이/서버에서만 존재)
	if (AShooterGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AShooterGameMode>() : nullptr)
	{
		// 웨이브 이벤트 구독. AddUniqueDynamic: 이미 등록돼 있으면 중복 등록하지 않는 버전
		GameMode->OnWaveStateChanged.AddUniqueDynamic(this, &UShooterHUDWidget::HandleWaveStateChanged);

		// 구독은 앞으로의 변화만 알려주므로 현재 값으로 한 번 표시
		HandleWaveStateChanged(GameMode->GetCurrentWave(), GameMode->GetEnemiesRemaining());
	}

	// 맞기 전에는 붉은 화면이 보이지 않아야 한다
	if (DamageOverlay)
	{
		DamageOverlay->SetRenderOpacity(0.f);
	}
}

void UShooterHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (DamageOverlay && CurrentDamageFlash > 0.f)
	{
		// FInterpConstantTo: 일정한 속도로 목표까지 이동 (Unity의 Mathf.MoveTowards). 속도 = 진하기 / 시간
		// SetRenderOpacity: 위젯 전체의 투명도. M_UI_DamageVignette가 이 값을 VertexColor의 알파로 받는다
		CurrentDamageFlash = FMath::FInterpConstantTo(CurrentDamageFlash, 0.f, InDeltaTime, DamageFlashOpacity / DamageFlashDuration);
		DamageOverlay->SetRenderOpacity(CurrentDamageFlash);
	}
}

void UShooterHUDWidget::HandleWaveStateChanged(int32 WaveNumber, int32 EnemiesRemaining)
{
	if (!WaveText)
	{
		return;
	}

	// 웨이브 시작 전(0)이면 준비 문구, 아니면 웨이브 번호와 남은 적 수
	const FString Text = (WaveNumber <= 0)
		? TEXT("GET READY")
		: FString::Printf(TEXT("WAVE %d  |  ENEMIES %d"), WaveNumber, EnemiesRemaining);
	WaveText->SetText(FText::FromString(Text));
}

void UShooterHUDWidget::BindToAbilitySystem(UAbilitySystemComponent* InASC)
{
	// 이미 다른 ASC를 구독 중이면 먼저 끊는다 (캐릭터가 바뀌는 경우 대비, 중복 구독 방지)
	UnbindFromAbilitySystem();

	BoundASC = InASC;
	if (!InASC)
	{
		return;
	}

	// 1) Health, MaxHealth가 바뀔 때마다 알림 받기
	//    AddUObject: "이 객체(this)의 이 멤버 함수를 불러줘" 등록. 반환값(번호표)을 저장해 두었다가 해제할 때 사용
	HealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UShooterHUDWidget::HandleHealthAttributeChanged);
	MaxHealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UShooterHUDWidget::HandleHealthAttributeChanged);

	// 2) State.Dead 태그가 붙거나 떨어질 때 알림 받기 (GameplayTag 이벤트 구독)
	//    NewOrRemoved: 태그가 "새로 생기거나(0→1) 완전히 사라질 때(1→0)"만 알림. 개수 변화마다 받는 AnyCountChange도 있다
	DeadTagHandle = InASC->RegisterGameplayTagEvent(ShooterTags::State_Dead, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UShooterHUDWidget::HandleDeadTagChanged);

	// 3) 탄약도 체력과 같은 방식으로 구독 (쏠 때, 무기 바꿀 때, 재장전할 때 바뀐다)
	AmmoChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetAmmoAttribute())
		.AddUObject(this, &UShooterHUDWidget::HandleAmmoAttributeChanged);
	MaxAmmoChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetMaxAmmoAttribute())
		.AddUObject(this, &UShooterHUDWidget::HandleAmmoAttributeChanged);

	// 4) 구독은 "앞으로의 변화"만 알려주므로, 현재 값으로 한 번 초기화
	RefreshHealth();
	RefreshAmmo();

	// 사망 텍스트는 처음엔 숨김 (이미 죽은 캐릭터에 연결된 경우는 보여줌)
	if (DeathText)
	{
		const bool bAlreadyDead = InASC->HasMatchingGameplayTag(ShooterTags::State_Dead);
		DeathText->SetVisibility(bAlreadyDead ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UShooterHUDWidget::NativeDestruct()
{
	// 위젯이 사라질 때 구독을 해제하지 않으면, ASC가 이미 없어진 위젯 함수를 부르려다 문제가 생길 수 있다
	UnbindFromAbilitySystem();

	// GameMode 웨이브 이벤트 구독도 해제 (C#의 event -=)
	if (AShooterGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AShooterGameMode>() : nullptr)
	{
		GameMode->OnWaveStateChanged.RemoveDynamic(this, &UShooterHUDWidget::HandleWaveStateChanged);
	}

	Super::NativeDestruct();
}

void UShooterHUDWidget::UnbindFromAbilitySystem()
{
	// Weak 포인터에서 실제 포인터 꺼내기. ASC가 이미 삭제됐으면 nullptr → 해제할 것도 없음
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		ASC->RegisterGameplayTagEvent(ShooterTags::State_Dead, EGameplayTagEventType::NewOrRemoved).Remove(DeadTagHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetAmmoAttribute()).Remove(AmmoChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(UShooterAttributeSet::GetMaxAmmoAttribute()).Remove(MaxAmmoChangedHandle);
	}

	// 번호표 초기화 (Reset = "아무것도 가리키지 않는 상태"로)
	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	DeadTagHandle.Reset();
	AmmoChangedHandle.Reset();
	MaxAmmoChangedHandle.Reset();
	BoundASC.Reset();
}

void UShooterHUDWidget::HandleHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	// Health와 MaxHealth 중 무엇이 바뀌었든 두 값을 다시 읽어서 갱신하면 된다 (단순하고 실수가 없음)
	RefreshHealth();

	// 이 함수는 MaxHealth 변화나 회복 때도 불리므로, Health가 줄었을 때만 붉은 화면을 켠다
	if (DamageOverlay && Data.Attribute == UShooterAttributeSet::GetHealthAttribute() && Data.NewValue < Data.OldValue)
	{
		CurrentDamageFlash = DamageFlashOpacity;
		DamageOverlay->SetRenderOpacity(CurrentDamageFlash);
	}
}

void UShooterHUDWidget::HandleDeadTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// NewCount > 0 → 태그가 붙음 = 방금 사망
	if (NewCount > 0)
	{
		if (DeathText)
		{
			// HitTestInvisible: 보이지만 마우스 클릭은 통과 (UI가 클릭을 가로채지 않게)
			DeathText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		// 추가 연출은 BP에 맡김
		OnOwnerDied();
	}
}

void UShooterHUDWidget::RefreshHealth()
{
	UAbilitySystemComponent* ASC = BoundASC.Get();
	if (!ASC)
	{
		return;
	}

	// GetNumericAttribute: ASC에 등록된 AttributeSet에서 해당 스탯의 현재값(CurrentValue)을 읽는다
	const float Health = ASC->GetNumericAttribute(UShooterAttributeSet::GetHealthAttribute());
	const float MaxHealth = ASC->GetNumericAttribute(UShooterAttributeSet::GetMaxHealthAttribute());

	// Progress Bar는 0.0 ~ 1.0 비율로 채워진다. MaxHealth가 0이면 나눗셈 오류 방지
	const float Percent = MaxHealth > 0.f ? Health / MaxHealth : 0.f;

	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}

	if (HealthText)
	{
		// FText::FromString: 화면에 표시할 텍스트 생성. FString::Printf는 C#의 string.Format과 같음
		// %.0f = 소수점 없이 표시 (80.0 → "80")
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}

	// BP 연출용 이벤트
	OnHealthUpdated(Health, MaxHealth);
}

void UShooterHUDWidget::HandleAmmoAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 체력과 같은 이유로 Ammo/MaxAmmo 중 무엇이 바뀌었든 둘 다 다시 읽는다
	RefreshAmmo();
}

void UShooterHUDWidget::RefreshAmmo()
{
	UAbilitySystemComponent* ASC = BoundASC.Get();
	if (!ASC || !AmmoText)
	{
		return;
	}

	const float Ammo = ASC->GetNumericAttribute(UShooterAttributeSet::GetAmmoAttribute());
	const float MaxAmmo = ASC->GetNumericAttribute(UShooterAttributeSet::GetMaxAmmoAttribute());
	AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Ammo, MaxAmmo)));

	// 탄약이 5발 이하면 빨간색으로 경고, 아니면 흰색
	// SetColorAndOpacity: 글자색 변경 (Unity의 text.color = Color.red 와 같음)
	// 탄약이 바뀔 때마다 이 함수가 불리므로 재장전으로 다시 채워지면 자동으로 흰색으로 돌아온다
	if (Ammo <= 5.f)
	{
		AmmoText->SetColorAndOpacity(FLinearColor::Red);
	}
	else
	{
		AmmoText->SetColorAndOpacity(FLinearColor::White);
	}
}
