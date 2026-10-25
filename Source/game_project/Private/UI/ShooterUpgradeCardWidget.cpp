#include "UI/ShooterUpgradeCardWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GAS/ShooterUpgradeData.h"
#include "Game/ShooterPlayerController.h"

void UShooterUpgradeCardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CardButton)
	{
		// 버튼이 눌리면 HandleCardClicked 호출 (C#의 button.onClick.AddListener(HandleCardClicked))
		CardButton->OnClicked.AddDynamic(this, &UShooterUpgradeCardWidget::HandleCardClicked);
	}
}

void UShooterUpgradeCardWidget::SetUpgrade(UShooterUpgradeData* InUpgrade)
{
	Upgrade = InUpgrade;
	if (!Upgrade)
	{
		return;
	}

	// 데이터 → 화면 반영. 위젯이 WBP에 없으면(nullptr) 그 줄은 건너뛴다
	if (NameText)
	{
		NameText->SetText(Upgrade->DisplayName);
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(Upgrade->Description);
	}
	if (IconImage)
	{
		if (Upgrade->Icon)
		{
			// SetBrushFromTexture: 이미지 위젯에 텍스처를 넣는다 (Unity의 image.sprite = ...)
			IconImage->SetBrushFromTexture(Upgrade->Icon);
			IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			// 아이콘이 아직 없으면 자리를 차지하지 않게 숨김 (Collapsed = 보이지 않고 공간도 차지 안 함)
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UShooterUpgradeCardWidget::HandleCardClicked()
{
	// GetOwningPlayer<T>(): 이 위젯을 만든 PlayerController를 T 타입으로 얻는다
	if (AShooterPlayerController* PC = GetOwningPlayer<AShooterPlayerController>())
	{
		PC->SelectUpgrade(Upgrade);
	}
}
