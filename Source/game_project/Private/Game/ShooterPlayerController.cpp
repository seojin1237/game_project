#include "Game/ShooterPlayerController.h"
#include "Game/ShooterGameMode.h"
#include "UI/ShooterHUDWidget.h"
#include "UI/ShooterUpgradeSelectWidget.h"
#include "Character/ShooterCharacterBase.h"
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

void AShooterPlayerController::ShowUpgradeSelection(const TArray<UShooterUpgradeData*>& Choices)
{
	// 선택 화면 위젯이 지정 안 됐으면 업그레이드 없이 바로 다음으로 진행 (게임이 멈춰버리지 않게)
	if (!UpgradeSelectWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Upgrade] UpgradeSelectWidgetClass가 비어 있습니다. 선택 없이 진행합니다."));
		if (AShooterGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
		{
			GameMode->NotifyUpgradeChosen();
		}
		return;
	}

	if (!UpgradeSelectWidget)
	{
		UpgradeSelectWidget = CreateWidget<UShooterUpgradeSelectWidget>(this, UpgradeSelectWidgetClass);
	}
	if (!UpgradeSelectWidget)
	{
		return;
	}

	UpgradeSelectWidget->SetChoices(Choices);

	// ZOrder 10: HUD(기본 0)보다 위에 그린다
	UpgradeSelectWidget->AddToViewport(10);
	bIsSelectingUpgrade = true;

	// 게임 일시정지: 적/타이머/물리 모두 멈춤. UI는 일시정지 중에도 계속 동작한다
	// (Unity의 Time.timeScale = 0 과 비슷)
	SetPause(true);
	EnterUIMode();
}

void AShooterPlayerController::SelectUpgrade(UShooterUpgradeData* Upgrade)
{
	// 이미 선택이 끝났으면 무시 (중복 클릭 방지)
	if (!bIsSelectingUpgrade)
	{
		return;
	}
	bIsSelectingUpgrade = false;

	// 1) 내 캐릭터에 업그레이드 적용
	if (AShooterCharacterBase* MyCharacter = Cast<AShooterCharacterBase>(GetPawn()))
	{
		MyCharacter->ApplyUpgrade(Upgrade);
	}

	// 2) 선택 화면 닫기 (RemoveFromParent = 화면에서 떼기. 인스턴스는 남아 있어 다음에 재사용)
	if (UpgradeSelectWidget)
	{
		UpgradeSelectWidget->RemoveFromParent();
	}

	// 3) 게임 재개 + 게임 입력 모드
	SetPause(false);
	EnterGameMode();

	// 4) GameMode에 알려서 다음 웨이브 진행
	if (AShooterGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		GameMode->NotifyUpgradeChosen();
	}
}

void AShooterPlayerController::EnterUIMode()
{
	// UI 전용 입력: 키보드/마우스가 게임(캐릭터 이동/사격)으로 가지 않고 UI(버튼 클릭)로만 간다
	FInputModeUIOnly InputMode;
	if (UpgradeSelectWidget)
	{
		// 키보드 포커스를 선택 화면에 줌 (나중에 키보드/패드로 카드 고르기를 넣을 때 필요)
		InputMode.SetWidgetToFocus(UpgradeSelectWidget->TakeWidget());
	}
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void AShooterPlayerController::EnterGameMode()
{
	// 게임 전용 입력: 다시 마우스로 시점 회전, 클릭으로 사격
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}
