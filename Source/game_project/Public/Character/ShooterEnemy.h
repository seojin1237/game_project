#pragma once

#include "CoreMinimal.h"
#include "Character/ShooterCharacterBase.h"
#include "ShooterEnemy.generated.h"

/**
 * 적 캐릭터. AI 컨트롤러가 조종한다.
 * 이동/공격 AI는 다음 단계에서 추가.
 */
UCLASS()
class GAME_PROJECT_API AShooterEnemy : public AShooterCharacterBase
{
	GENERATED_BODY()

public:
	AShooterEnemy();

protected:
	// 부모의 사망 처리 + 일정 시간 뒤 삭제
	virtual void Die() override;

	// 사망 후 액터가 사라지기까지 걸리는 시간(초). 0이면 사라지지 않음
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Enemy" 그룹으로 표시
	// meta=(ClampMin)  : 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Enemy", meta = (ClampMin = "0"))
	float DeathLifeSpan = 3.f;
};
