#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUpgradeCardWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UShooterUpgradeData;

/**
 * 업그레이드 카드 1장 (WBP_UpgradeCard의 C++ 부모).
 *  - SetUpgrade()로 받은 데이터의 이름/설명/아이콘을 표시
 *  - 카드를 클릭하면 PlayerController에 "이걸 골랐다"고 알림
 *
 * 겉모습(크기, 색, 폰트)은 WBP에서, 클릭 로직은 여기서.
 * Abstract : 디자인 없이 단독으로 쓰지 않음 (반드시 WBP를 만들어 사용)
 */
UCLASS(Abstract)
class GAME_PROJECT_API UShooterUpgradeCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이 카드가 보여줄 업그레이드를 지정하고 화면 내용을 채운다
	void SetUpgrade(UShooterUpgradeData* InUpgrade);

protected:
	// 위젯이 처음 만들어질 때 한 번만 호출 (Unity의 Awake와 비슷) → 버튼 클릭 이벤트 연결
	// NativeConstruct는 화면에 붙을 때마다 호출될 수 있어서, 이벤트 연결은 한 번만 되는 이쪽에 둔다
	virtual void NativeOnInitialized() override;

	// 카드 버튼 클릭 시 호출
	// UFUNCTION() : 버튼의 OnClicked(다이내믹 델리게이트)에 AddDynamic으로 연결하려면 필요
	UFUNCTION()
	void HandleCardClicked();

	// 카드 전체를 덮는 버튼. WBP에 "CardButton"이라는 이름의 Button이 반드시 있어야 한다
	// BindWidget : WBP 디자이너의 같은 이름 위젯과 자동 연결 (필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CardButton;

	// 업그레이드 이름 표시 (선택)
	// BindWidgetOptional : 같은 이름 위젯이 있으면 연결, 없으면 nullptr
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	// 업그레이드 설명 표시 (선택, 지정자 의미는 위와 동일)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;

	// 업그레이드 아이콘 (선택, 지정자 의미는 위와 동일). 데이터에 아이콘이 없으면 숨긴다
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

private:
	// 이 카드가 나타내는 업그레이드
	// UPROPERTY() : 가비지 컬렉터가 추적하도록 (UObject 포인터 멤버엔 필수)
	UPROPERTY()
	TObjectPtr<UShooterUpgradeData> Upgrade;
};
