#include "GAS/ShooterGameplayEffectContext.h"

FShooterGameplayEffectContext* FShooterGameplayEffectContext::ExtractEffectContext(FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseContext = Handle.Get();

	// IsChildOf: "이 타입이 우리 타입(또는 그 자식)인가" (C#의 `is FShooterGameplayEffectContext`)
	if (BaseContext && BaseContext->GetScriptStruct()->IsChildOf(FShooterGameplayEffectContext::StaticStruct()))
	{
		return static_cast<FShooterGameplayEffectContext*>(BaseContext);
	}
	return nullptr;
}

FShooterGameplayEffectContext* FShooterGameplayEffectContext::Duplicate() const
{
	// 우리 타입으로 새로 만들고 모든 필드 복사 (*this 대입 = 멤버 전체 복사)
	FShooterGameplayEffectContext* NewContext = new FShooterGameplayEffectContext();
	*NewContext = *this;

	// HitResult는 공유 포인터라 위 복사로는 "같은 것"을 가리키게 된다 → 엔진 기본 구현처럼 깊은 복사
	if (GetHitResult())
	{
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

bool FShooterGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// 부모 필드(가해자, HitResult 등) 먼저 직렬화
	Super::NetSerialize(Ar, Map, bOutSuccess);

	// 우리 필드: bool 하나라 1비트만 사용 (저장 시엔 값을 쓰고, 읽을 땐 값이 채워진다)
	uint8 bCriticalBit = bIsCriticalHit ? 1 : 0;
	Ar.SerializeBits(&bCriticalBit, 1);
	if (Ar.IsLoading())
	{
		bIsCriticalHit = (bCriticalBit != 0);
	}

	return true;
}
