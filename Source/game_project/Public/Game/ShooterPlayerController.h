#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UShooterHUDWidget;

/**
 * 플레이어 컨트롤러 = "화면 앞에 앉은 플레이어" 그 자체.
 * 캐릭터(몸)가 죽거나 바뀌어도 컨트롤러는 남아 있으므로, HUD/메뉴/게임오버 화면 같은 UI를 여기서 관리한다.
 */
UCLASS()
class GAME_PROJECT_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// 캐릭터에 빙의할 때 호출 → HUD를 만들고(처음 한 번) 그 캐릭터의 ASC에 연결
	virtual void OnPossess(APawn* InPawn) override;

	// 화면에 띄울 HUD 위젯 클래스 (BP에서 WBP_HUD 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "UI" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShooterHUDWidget> HUDWidgetClass;

	// 실제로 만들어진 HUD 위젯 인스턴스
	// UPROPERTY() (지정자 없음) : 에디터에 노출하지 않지만, 가비지 컬렉터가 이 포인터를 추적하게 해서
	//                            사용 중인 위젯이 메모리에서 지워지지 않도록 보호한다 (UObject 포인터 멤버엔 필수)
	UPROPERTY()
	TObjectPtr<UShooterHUDWidget> HUDWidget;
};
