#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UShooterHUDWidget;
class UShooterUpgradeSelectWidget;
class UShooterUpgradeData;

/**
 * 플레이어 컨트롤러 = "화면 앞에 앉은 플레이어" 그 자체.
 * 캐릭터(몸)가 죽거나 바뀌어도 컨트롤러는 남아 있으므로, HUD/메뉴/게임오버 화면 같은 UI를 여기서 관리한다.
 */
UCLASS()
class GAME_PROJECT_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 업그레이드 선택 화면 열기: 게임 일시정지 + 마우스 커서 표시 + UI 입력 모드 (GameMode가 웨이브 클리어 시 호출)
	void ShowUpgradeSelection(const TArray<UShooterUpgradeData*>& Choices);

	// 카드를 골랐을 때 호출: 업그레이드 적용 → 화면 닫기 → 게임 재개 → GameMode에 알림
	// BlueprintCallable : 블루프린트(WBP)에서도 호출 가능 (C++ 카드 위젯이 기본으로 호출함)
	// Category          : "Upgrade" 그룹으로 표시
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void SelectUpgrade(UShooterUpgradeData* Upgrade);

protected:
	// 캐릭터에 빙의할 때 호출 → HUD를 만들고(처음 한 번) 그 캐릭터의 ASC에 연결
	virtual void OnPossess(APawn* InPawn) override;

	// 화면에 띄울 HUD 위젯 클래스 (BP에서 WBP_HUD 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "UI" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShooterHUDWidget> HUDWidgetClass;

	// 업그레이드 선택 화면 위젯 클래스 (BP에서 WBP_UpgradeSelect 지정) (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShooterUpgradeSelectWidget> UpgradeSelectWidgetClass;

	// 실제로 만들어진 HUD 위젯 인스턴스
	// UPROPERTY() (지정자 없음) : 에디터에 노출하지 않지만, 가비지 컬렉터가 이 포인터를 추적하게 해서
	//                            사용 중인 위젯이 메모리에서 지워지지 않도록 보호한다 (UObject 포인터 멤버엔 필수)
	UPROPERTY()
	TObjectPtr<UShooterHUDWidget> HUDWidget;

	// 업그레이드 선택 화면 인스턴스 (한 번 만들고 재사용) (지정자 의미는 위와 동일)
	UPROPERTY()
	TObjectPtr<UShooterUpgradeSelectWidget> UpgradeSelectWidget;

private:
	// 선택 화면이 열려 있는지. 카드를 빠르게 두 번 눌러 업그레이드가 두 번 적용되는 것을 막는다
	bool bIsSelectingUpgrade = false;

	// 입력 모드 전환 도우미
	void EnterUIMode();
	void EnterGameMode();
};
