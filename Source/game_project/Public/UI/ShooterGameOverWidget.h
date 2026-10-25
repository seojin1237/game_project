#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterGameOverWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 게임 오버 화면 (WBP_GameOver의 C++ 부모).
 *  - 도달한 웨이브 등 결과 표시
 *  - RETRY: 처음부터 다시 / QUIT: 게임 종료
 *
 * 버튼이 하는 일(로직)은 PlayerController에 맡기고, 이 위젯은 "누가 눌렸는지"만 전달한다.
 * Abstract : 디자인 없이 단독으로 쓰지 않음 (반드시 WBP를 만들어 사용)
 */
UCLASS(Abstract)
class GAME_PROJECT_API UShooterGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 결과 표시 내용 채우기
	void SetResult(int32 WaveReached);

protected:
	// 위젯이 처음 만들어질 때 한 번만 호출 → 버튼 클릭 이벤트 연결 (중복 연결 방지를 위해 여기서)
	virtual void NativeOnInitialized() override;

	// 버튼 클릭 처리
	// UFUNCTION() : 버튼 OnClicked(다이내믹 델리게이트)에 AddDynamic으로 연결하려면 필요
	UFUNCTION()
	void HandleRetryClicked();

	UFUNCTION()
	void HandleQuitClicked();

	// [BP 확장용] 결과가 세팅될 때 호출. 등장 애니메이션 재생 등 연출용
	// BlueprintImplementableEvent : C++은 선언만, 몸체는 BP 이벤트 그래프에서 구현 (구현 안 하면 아무 일 없음)
	// Category                    : "GameOver" 그룹으로 표시
	UFUNCTION(BlueprintImplementableEvent, Category = "GameOver")
	void OnResultShown(int32 WaveReached);

	// 다시 시작 버튼. WBP에 "RetryButton"이 반드시 있어야 한다
	// BindWidget : WBP 디자이너의 같은 이름 위젯과 자동 연결 (필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RetryButton;

	// 게임 종료 버튼 (선택)
	// BindWidgetOptional : 같은 이름 위젯이 있으면 연결, 없으면 nullptr
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	// "WAVE 3 REACHED" 같은 결과 텍스트 (선택, 지정자 의미는 위와 동일)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultText;
};
