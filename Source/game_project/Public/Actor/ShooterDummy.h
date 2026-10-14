#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "ShooterDummy.generated.h"

class UStaticMeshComponent;
class UAbilitySystemComponent;
class UShooterAttributeSet;
struct FOnAttributeChangeData;

/**
 * 사격 테스트용 과녁.
 * 캐릭터와 똑같이 ASC + AttributeSet을 가지므로 GE_Damage를 맞을 수 있다.
 * 나중에 적(Enemy) 클래스를 만들 때 이 구조를 그대로 가져간다.
 */
UCLASS()
class GAME_PROJECT_API AShooterDummy : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AShooterDummy();

	// IAbilitySystemInterface 구현: 사격 어빌리티가 이 함수로 과녁의 ASC를 찾는다
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

	// ASC가 Health 값이 바뀔 때마다 호출해주는 함수 (BeginPlay에서 등록)
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// 사망 로직 (C++): 태그 부여, 충돌 끄기, 일정 시간 뒤 삭제
	void Die();

	// 사망 연출 (BP): 이펙트/사운드/머티리얼 변경 등은 BP 이벤트 그래프에서 구현
	// BlueprintImplementableEvent : C++에는 선언만 있고 몸체는 BP에서 만든다 (C++에서 호출 가능)
	// Category                    : "Dummy" 그룹으로 표시
	UFUNCTION(BlueprintImplementableEvent, Category = "Dummy")
	void OnDeath();

	// 사망 후 액터가 사라지기까지 걸리는 시간(초). 0이면 사라지지 않음
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Dummy" 그룹으로 표시
	// meta=(ClampMin)  : 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Dummy", meta = (ClampMin = "0"))
	float DeathLifeSpan = 2.f;

	// 눈에 보이는 몸체 + 총알이 맞는 충돌체 (BP에서 Cube 등 메시 지정)
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(메시, 머티리얼 등)은 수정 가능, 컴포넌트 교체는 불가
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Dummy" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dummy")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// GAS 본체 (지정자 의미는 위와 동일, Category만 "GAS")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 체력 스탯 (캐릭터와 같은 AttributeSet 재사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UShooterAttributeSet> AttributeSet;
};
