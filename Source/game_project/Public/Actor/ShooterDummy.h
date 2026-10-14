#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "ShooterDummy.generated.h"

class UStaticMeshComponent;
class UAbilitySystemComponent;
class UShooterAttributeSet;

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
