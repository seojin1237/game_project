#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "ShooterGameplayEffectContext.generated.h"

/**
 * 우리 게임 전용 Effect Context.
 *
 * Effect Context란?
 * - GE 하나가 적용될 때 따라다니는 "부가 정보 꾸러미". 기본으로 가해자(Instigator), 맞은 위치(HitResult) 등이 들어 있다.
 * - GE → Execution → GameplayCue까지 같은 꾸러미가 전달되므로,
 *   Execution에서 "치명타였다"라고 적어두면 Cue(데미지 숫자)에서 그걸 읽을 수 있다.
 *   (C#으로 치면 이벤트 인자 클래스 EventArgs를 상속해서 필드를 하나 더 붙인 것)
 *
 * 이 구조체를 실제로 쓰게 하려면 UShooterAbilitySystemGlobals가 "기본 Context 대신 이걸 만들어라"라고 알려줘야 한다.
 *
 * USTRUCT() : 언리얼 리플렉션에 등록되는 구조체 (복사/네트워크 직렬화 등 엔진 기능 사용 가능)
 */
USTRUCT()
struct GAME_PROJECT_API FShooterGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	bool IsCriticalHit() const { return bIsCriticalHit; }
	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }

	// Handle 안에 들어 있는 Context가 우리 타입이면 꺼내준다 (아니면 nullptr)
	// 다른 종류 Context가 섞여 들어와도 잘못 캐스팅하지 않도록 타입을 확인한다
	static FShooterGameplayEffectContext* ExtractEffectContext(FGameplayEffectContextHandle Handle);

	// ===== 엔진이 Context를 다룰 때 호출하는 함수들 (상속한 Context라면 반드시 재정의) =====

	// "나는 어떤 구조체 타입인가" → 엔진이 타입 확인/직렬화에 사용
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	// 복제본 생성. 기본 구현은 부모 타입으로만 복사하므로 우리 필드(bIsCriticalHit)가 사라지지 않게 재정의
	virtual FShooterGameplayEffectContext* Duplicate() const override;

	// 네트워크 전송용 직렬화. 지금은 싱글플레이지만 엔진 규약상 구현해 둔다
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

protected:
	// 이번 공격이 치명타였는지 (UShooterDamageExecution이 기록)
	// UPROPERTY() : 지정자 없이 리플렉션에만 등록 (복사/GC 대상으로 인식되게. 에디터/BP 노출 없음)
	UPROPERTY()
	bool bIsCriticalHit = false;
};

/**
 * 구조체 특성(Traits) 등록: "이 구조체는 직접 만든 NetSerialize와 복사를 지원한다"고 엔진에 알려준다.
 * 이게 없으면 엔진이 우리 NetSerialize를 호출하지 않는다.
 */
template<>
struct TStructOpsTypeTraits<FShooterGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FShooterGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
