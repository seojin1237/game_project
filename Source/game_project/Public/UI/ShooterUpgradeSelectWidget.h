#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUpgradeSelectWidget.generated.h"

class UPanelWidget;
class UShooterUpgradeCardWidget;
class UShooterUpgradeData;

/**
 * 업그레이드 선택 화면 (WBP_UpgradeSelect의 C++ 부모).
 * 받은 업그레이드 목록만큼 카드 위젯(WBP_UpgradeCard)을 만들어 CardContainer 안에 나열한다.
 * Abstract : 디자인 없이 단독으로 쓰지 않음 (반드시 WBP를 만들어 사용)
 */
UCLASS(Abstract)
class GAME_PROJECT_API UShooterUpgradeSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 보여줄 업그레이드 후보들로 카드를 새로 만든다 (이전 카드는 모두 제거)
	void SetChoices(const TArray<UShooterUpgradeData*>& Choices);

protected:
	// 카드들을 담을 패널. WBP에 "CardContainer"라는 이름의 패널(Horizontal Box 권장)이 반드시 있어야 한다
	// BindWidget : WBP 디자이너의 같은 이름 위젯과 자동 연결 (필수)
	// UPanelWidget : 자식 위젯을 담을 수 있는 모든 패널의 부모 타입 (HorizontalBox, VerticalBox 등)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> CardContainer;

	// 카드 1장으로 만들 위젯 클래스 (WBP_UpgradeCard 지정)
	// EditDefaultsOnly : 블루프린트(WBP) 클래스 기본값에서만 지정 가능
	// Category         : "Upgrade" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	TSubclassOf<UShooterUpgradeCardWidget> CardWidgetClass;

	// 카드 사이 간격 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	float CardSpacing = 20.f;
};
