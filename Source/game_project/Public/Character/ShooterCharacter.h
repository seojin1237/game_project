#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ShooterCharacter.generated.h"

class UAbilitySystemComponent;
class UShooterAttributeSet;
class UGameplayEffect;

/**
 * 플레이어 캐릭터.
 * GAS를 쓰기 위해 AbilitySystemComponent(ASC)를 가지고,
 * IAbilitySystemInterface로 "나는 ASC를 가지고 있다"고 외부에 알려준다.
 */
UCLASS()
class GAME_PROJECT_API AShooterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	// IAbilitySystemInterface 구현: GAS 시스템이 이 함수로 캐릭터의 ASC를 찾아간다
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// [테스트용] 콘솔(~ 키)에 DebugDamage 입력 시 DebugDamageEffect를 자기 자신에게 적용
	// Exec : 콘솔 명령어로 호출 가능 (PlayerController, 빙의된 Pawn, GameMode 등에서만 동작)
	UFUNCTION(Exec)
	void DebugDamage();

protected:
	// 컨트롤러가 이 캐릭터에 빙의(Possess)했을 때 호출된다
	virtual void PossessedBy(AController* NewController) override;

	// VisibleAnywhere   : 에디터 디테일 패널에서 보이기만 함 (컴포넌트 자체를 다른 걸로 교체 불가)
	// BlueprintReadOnly : 블루프린트에서 읽기(Get)만 가능, 대입(Set) 불가
	// Category          : 디테일 패널/블루프린트에서 "GAS" 그룹으로 묶어서 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 체력 등 스탯 묶음 (지정자 의미는 위와 동일)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UShooterAttributeSet> AttributeSet;

	// [테스트용] DebugDamage에서 적용할 GameplayEffect 클래스 (BP에서 GE_Damage 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능 (레벨에 배치된 개별 인스턴스에선 수정 불가)
	// Category         : "GAS|Debug" 하위 그룹으로 표시 (| 는 그룹 안의 그룹)
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Debug")
	TSubclassOf<UGameplayEffect> DebugDamageEffect;
};
