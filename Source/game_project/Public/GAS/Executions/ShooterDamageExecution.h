#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ShooterDamageExecution.generated.h"

/**
 * 데미지 계산식 (GE_Damage의 Executions에 지정).
 *
 * Execution Calculation이란?
 * - GE의 Modifier는 "속성 하나를 읽어 곱하고 더하기" 정도만 할 수 있다.
 * - 확률 판정(치명타), 방어력 차감처럼 "여러 속성을 읽고 조건을 따지는" 계산은 C++ 함수로 직접 짠다.
 *   그게 Execution Calculation. GE가 적용될 때 GAS가 Execute()를 호출해 준다.
 *   (C#으로 치면 데미지 공식을 담은 static 함수를 GE에 꽂아두는 것)
 * - 수치(공격력, 치명타 확률/배율)는 여전히 AttributeSet 값이고, 그 값은 GE/업그레이드로 BP에서 조절한다.
 *   여기는 "공식"만 담당한다.
 *
 * 현재 공식: 데미지 = 공격자 AttackPower × (치명타면 CritMultiplier, 아니면 1)
 *            → 대상 Health에서 뺀다
 */
UCLASS()
class GAME_PROJECT_API UShooterDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UShooterDamageExecution();

	// GE가 적용될 때 GAS가 호출. 읽은 속성으로 계산해서 OutExecutionOutput에 "Health를 얼마 바꿔라"를 넣는다
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
