#include "Game/ShooterPlayerController.h"
#include "UI/ShooterHUDWidget.h"
#include "AbilitySystemBlueprintLibrary.h"

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	// 부모 처리 안에서 캐릭터의 PossessedBy가 호출됨 → 캐릭터 ASC 초기화(InitAbilityActorInfo)가 끝난 뒤에 아래가 실행된다
	Super::OnPossess(InPawn);

	// 이 PC 앞에 실제 사람이 있을 때만 UI를 만든다 (멀티플레이 서버의 남의 컨트롤러 등은 제외)
	if (!IsLocalController() || !HUDWidgetClass)
	{
		return;
	}

	// HUD는 한 번만 만들고 계속 재사용 (리스폰해도 같은 HUD에 새 캐릭터만 다시 연결)
	if (!HUDWidget)
	{
		// CreateWidget<타입>(소유 PC, 위젯 클래스): 위젯 인스턴스 생성 (Unity의 Instantiate(uiPrefab)과 비슷)
		HUDWidget = CreateWidget<UShooterHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			// 화면에 붙이기 (Unity에서 Canvas 아래에 UI를 활성화하는 것과 같은 단계)
			HUDWidget->AddToViewport();
		}
	}

	if (HUDWidget)
	{
		// 빙의한 캐릭터의 ASC를 찾아서 HUD에 연결 (IAbilitySystemInterface를 통해 찾음)
		HUDWidget->BindToAbilitySystem(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InPawn));
	}
}
