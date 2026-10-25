#include "UI/ShooterUpgradeSelectWidget.h"
#include "UI/ShooterUpgradeCardWidget.h"
#include "Components/PanelWidget.h"
#include "Components/HorizontalBoxSlot.h"

void UShooterUpgradeSelectWidget::SetChoices(const TArray<UShooterUpgradeData*>& Choices)
{
	if (!CardContainer || !CardWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Upgrade] CardContainer 또는 CardWidgetClass가 비어 있습니다 (WBP_UpgradeSelect 확인)"));
		return;
	}

	// 지난 선택 때 만든 카드 제거 (Unity에서 자식 오브젝트를 전부 Destroy하는 것과 같음)
	CardContainer->ClearChildren();

	for (UShooterUpgradeData* Choice : Choices)
	{
		// 카드 위젯 생성 → 데이터 채우기 → 컨테이너에 추가
		UShooterUpgradeCardWidget* Card = CreateWidget<UShooterUpgradeCardWidget>(GetOwningPlayer(), CardWidgetClass);
		if (!Card)
		{
			continue;
		}
		Card->SetUpgrade(Choice);

		// AddChild는 "슬롯"을 돌려준다. 슬롯 = 부모 패널 안에서의 배치 정보(여백, 정렬 등)
		// HorizontalBox에 넣었다면 HorizontalBoxSlot이므로, 캐스트해서 카드 사이 간격을 준다
		UPanelSlot* PanelSlot = CardContainer->AddChild(Card);
		if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(PanelSlot))
		{
			HBoxSlot->SetPadding(FMargin(CardSpacing * 0.5f, 0.f));
		}
	}
}
