#include "Game/ShooterPlayerController.h"
#include "Game/ShooterGameMode.h"
#include "UI/ShooterHUDWidget.h"
#include "UI/ShooterUpgradeSelectWidget.h"
#include "UI/ShooterGameOverWidget.h"
#include "Character/ShooterCharacterBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	// 부모 처리 안에서 캐릭터의 PossessedBy가 호출됨 → 캐릭터 ASC 초기화(InitAbilityActorInfo)가 끝난 뒤에 아래가 실행된다
	Super::OnPossess(InPawn);

	// 내 캐릭터가 죽으면 HandlePawnDied를 불러달라고 구독 (게임 오버 화면용)
	// AddUniqueDynamic: 같은 함수가 이미 등록돼 있으면 또 등록하지 않음 (재빙의 시 중복 방지)
	if (AShooterCharacterBase* MyCharacter = Cast<AShooterCharacterBase>(InPawn))
	{
		MyCharacter->OnCharacterDied.AddUniqueDynamic(this, &AShooterPlayerController::HandlePawnDied);
	}

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
	EnterUIMode(UpgradeSelectWidget);
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

void AShooterPlayerController::HandlePawnDied(AShooterCharacterBase* DeadCharacter)
{
	// 바로 띄우지 않고 GameOverDelay초 뒤에 (그동안 HUD의 YOU DIED가 보인다)
	GetWorldTimerManager().SetTimer(GameOverTimerHandle, this, &AShooterPlayerController::ShowGameOver, GameOverDelay, false);
}

void AShooterPlayerController::ShowGameOver()
{
	if (!IsLocalController() || !GameOverWidgetClass)
	{
		return;
	}

	if (!GameOverWidget)
	{
		GameOverWidget = CreateWidget<UShooterGameOverWidget>(this, GameOverWidgetClass);
	}
	if (!GameOverWidget)
	{
		return;
	}

	// 도달한 웨이브 = GameMode의 현재 웨이브 번호
	int32 WaveReached = 0;
	if (const AShooterGameMode* GameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		WaveReached = GameMode->GetCurrentWave();
	}
	GameOverWidget->SetResult(WaveReached);

	// HUD는 숨긴다 (HUD의 YOU DIED 글자가 게임 오버 화면 뒤에 겹쳐 보이지 않게)
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	// ZOrder 20: HUD(0), 업그레이드 선택(10)보다 위
	GameOverWidget->AddToViewport(20);
	EnterUIMode(GameOverWidget);
}

void AShooterPlayerController::RestartRun()
{
	// 현재 레벨 이름을 얻어서 다시 연다 (Unity의 SceneManager.LoadScene(현재 씬 이름))
	// 레벨을 새로 열면 모든 액터/위젯/타이머가 새로 만들어지므로 따로 초기화할 게 없다
	// bRemovePrefixString=true: 에디터 플레이 중 붙는 "UEDPIE_0_" 같은 접두사를 제거한 순수 이름
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

void AShooterPlayerController::QuitRun()
{
	// 게임 종료. 에디터에서 플레이 중이면 플레이만 끝난다
	// (EQuitPreference::Quit = 완전 종료, Background = 모바일에서 백그라운드로)
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AShooterPlayerController::EnterUIMode(UUserWidget* FocusWidget)
{
	// UI 전용 입력: 키보드/마우스가 게임(캐릭터 이동/사격)으로 가지 않고 UI(버튼 클릭)로만 간다
	FInputModeUIOnly InputMode;
	if (FocusWidget)
	{
		// 위젯은 기본적으로 "포커스를 받을 수 없음" 상태라, 먼저 받을 수 있게 켜야 한다
		// (안 켜면 "Attempting to focus Non-Focusable widget" 에러 로그가 찍힌다)
		FocusWidget->SetIsFocusable(true);

		// 키보드 포커스를 해당 화면에 줌 (나중에 키보드/패드로 버튼을 고를 때 필요)
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
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
