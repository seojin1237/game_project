#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterAIController.generated.h"

/**
 * 적을 조종하는 AI 컨트롤러 (1단계: 플레이어 추적만).
 *
 * 플레이어 컨트롤러가 "키보드/마우스 입력 → 캐릭터 조종"이라면,
 * AI 컨트롤러는 "코드가 판단 → 캐릭터 조종"을 한다. 캐릭터(몸)와 컨트롤러(두뇌)가 분리된 구조.
 *
 * 지금은 일정 간격으로 "플레이어에게 가라"고 명령만 반복한다.
 * 공격/순찰처럼 판단이 복잡해지면 Behavior Tree로 옮길 예정.
 */
UCLASS()
class GAME_PROJECT_API AShooterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AShooterAIController();

protected:
	// 이 컨트롤러가 캐릭터에 빙의(Possess)했을 때 호출 → 추적 타이머 시작
	virtual void OnPossess(APawn* InPawn) override;

	// 빙의가 풀렸을 때(캐릭터가 삭제될 때 등) 호출 → 타이머 정리
	virtual void OnUnPossess() override;

	// 타이머가 주기적으로 부르는 함수: 플레이어 위치로 다시 이동 명령
	void UpdateChase();

	// 몇 초마다 이동 목표를 갱신할지. 너무 짧으면 낭비, 너무 길면 플레이어를 늦게 따라온다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "AI" 그룹으로 표시
	// meta=(ClampMin)  : 0.05초 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "AI", meta = (ClampMin = "0.05"))
	float ChaseUpdateInterval = 0.25f;

	// 플레이어와 이 거리(cm) 안에 들어오면 멈춘다 (나중에 공격 사거리로 사용)
	// 지정자 의미는 위와 동일, ClampMin = 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "AI", meta = (ClampMin = "0"))
	float AcceptanceRadius = 150.f;

private:
	// SetTimer가 돌려주는 "타이머 번호표". 나중에 이걸로 타이머를 멈춘다
	// (Unity의 StartCoroutine이 돌려주는 Coroutine 객체와 비슷한 역할)
	FTimerHandle ChaseTimerHandle;
};
