#include "GAS/Executions/ShooterDamageExecution.h"
#include "GAS/ShooterAttributeSet.h"
#include "GAS/ShooterGameplayEffectContext.h"

namespace
{
	/**
	 * "어떤 속성을, 누구(Source/Target)에게서, 언제 읽을지" 정의(Capture Definition) 모음.
	 * GAS는 계산에 쓸 속성을 미리 등록(캡처)해 둬야 Execute 안에서 값을 읽게 해준다.
	 * 한 번만 만들면 되므로 함수 안 static 변수로 둔다 (C#의 static readonly 필드와 비슷)
	 */
	struct FShooterDamageStatics
	{
		FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
		FGameplayEffectAttributeCaptureDefinition CritChanceDef;
		FGameplayEffectAttributeCaptureDefinition CritMultiplierDef;

		FShooterDamageStatics()
		{
			// Source = 공격자(GE를 건 쪽)에게서 읽는다
			// bSnapshot = true : Spec을 만든 순간(= 총을 쏜 순간)의 값으로 고정
			//   → 맞기 직전에 버프가 바뀌어도 "쏠 때의 공격력"이 적용된다
			AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
				UShooterAttributeSet::GetAttackPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
			CritChanceDef = FGameplayEffectAttributeCaptureDefinition(
				UShooterAttributeSet::GetCritChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
			CritMultiplierDef = FGameplayEffectAttributeCaptureDefinition(
				UShooterAttributeSet::GetCritMultiplierAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
		}
	};

	const FShooterDamageStatics& DamageStatics()
	{
		static FShooterDamageStatics Statics;
		return Statics;
	}
}

UShooterDamageExecution::UShooterDamageExecution()
{
	// 이 계산이 읽을 속성들을 GAS에 등록
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritMultiplierDef);
}

void UShooterDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 속성 값을 계산할 때 쓰는 태그 조건 (태그에 따라 켜지는 버프 등이 있으면 여기서 반영된다)
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 캡처한 속성 값 읽기 (out 매개변수에 값이 채워진다. 실패하면 초기값 그대로)
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvalParams, AttackPower);

	float CritChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritChanceDef, EvalParams, CritChance);

	float CritMultiplier = 1.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritMultiplierDef, EvalParams, CritMultiplier);

	float Damage = FMath::Max(AttackPower, 0.f);

	// 치명타 판정. FRand(): 0 ~ 1 사이 랜덤 실수 (Unity의 Random.value)
	const bool bCritical = FMath::FRand() < FMath::Clamp(CritChance, 0.f, 1.f);
	if (bCritical)
	{
		Damage *= FMath::Max(CritMultiplier, 1.f);
		UE_LOG(LogTemp, Log, TEXT("[GAS] Critical hit! x%.1f -> %.1f"), CritMultiplier, Damage);
	}

	// 치명타 여부를 Effect Context에 기록 → 같은 Context를 받는 GameplayCue(데미지 숫자)가 읽을 수 있다
	// Spec은 const지만 Context는 공유 포인터(Handle)라 복사해서 꺼내면 내용을 바꿀 수 있다
	if (FShooterGameplayEffectContext* Context = FShooterGameplayEffectContext::ExtractEffectContext(Spec.GetContext()))
	{
		Context->SetIsCriticalHit(bCritical);
	}

	if (Damage <= 0.f)
	{
		return;
	}

	// 결과 출력: "대상의 Health에 -Damage를 더해라"
	// 실제 적용은 GAS가 하고, 이후 AttributeSet의 PostGameplayEffectExecute(0 ~ MaxHealth 제한)도 그대로 거친다
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UShooterAttributeSet::GetHealthAttribute(), EGameplayModOp::AddBase, -Damage));
}
