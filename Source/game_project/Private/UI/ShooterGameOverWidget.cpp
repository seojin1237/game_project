#include "UI/ShooterGameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Game/ShooterPlayerController.h"

void UShooterGameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 버튼 클릭 → 처리 함수 연결 (C#의 button.onClick.AddListener)
	if (RetryButton)
	{
		RetryButton->OnClicked.AddDynamic(this, &UShooterGameOverWidget::HandleRetryClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UShooterGameOverWidget::HandleQuitClicked);
	}
}

void UShooterGameOverWidget::SetResult(int32 WaveReached)
{
	if (ResultText)
	{
		ResultText->SetText(FText::FromString(FString::Printf(TEXT("WAVE %d REACHED"), WaveReached)));
	}

	// BP 연출용 이벤트
	OnResultShown(WaveReached);
}

void UShooterGameOverWidget::HandleRetryClicked()
{
	// 실제 재시작 로직은 PlayerController 담당 (위젯은 "눌렸다"만 전달)
	if (AShooterPlayerController* PC = GetOwningPlayer<AShooterPlayerController>())
	{
		PC->RestartRun();
	}
}

void UShooterGameOverWidget::HandleQuitClicked()
{
	if (AShooterPlayerController* PC = GetOwningPlayer<AShooterPlayerController>())
	{
		PC->QuitRun();
	}
}
