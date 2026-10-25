#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ShooterCharacterBase.generated.h"

class UAbilitySystemComponent;
class UShooterAttributeSet;
class UGameplayEffect;
class AShooterCharacterBase;
struct FOnAttributeChangeData;

/**
 * 캐릭터가 죽었을 때 알려주는 이벤트 타입 (GameMode가 웨이브 남은 적 수를 세는 데 사용).
 * DYNAMIC_MULTICAST : 여러 구독자 가능 + 블루프린트에서도 바인딩 가능 (C#의 event Action<Character>)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDiedSignature, AShooterCharacterBase*, DeadCharacter);

/**
 * 플레이어와 적이 공통으로 상속하는 캐릭터 베이스.
 * ASC + AttributeSet을 가지고, 체력이 0이 되면 사망 처리를 한다.
 * Abstract : 이 클래스 자체는 레벨에 배치하거나 BP 부모로 직접 쓰지 않는다 (자식만 사용)
 */
UCLASS(Abstract)
class GAME_PROJECT_API AShooterCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AShooterCharacterBase();

	// IAbilitySystemInterface 구현: GAS 시스템이 이 함수로 캐릭터의 ASC를 찾아간다
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 죽었는지 여부 (State.Dead 태그 보유 여부)
	// BlueprintPure : 실행 핀 없는 "값만 돌려주는" BP 노드로 만든다 (Get 노드처럼)
	// Category      : "Character" 그룹으로 표시
	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsDead() const;

	// 사망 순간에 한 번 호출되는 이벤트. 외부(GameMode 등)가 구독한다
	// BlueprintAssignable : 블루프린트에서 "Bind Event to On Character Died"로 연결 가능
	// Category            : "Character" 그룹으로 표시
	UPROPERTY(BlueprintAssignable, Category = "Character")
	FOnCharacterDiedSignature OnCharacterDied;

protected:
	virtual void BeginPlay() override;

	// 컨트롤러(플레이어 또는 AI)가 빙의했을 때 호출 → ASC 초기화
	virtual void PossessedBy(AController* NewController) override;

	// ASC가 Health 값이 바뀔 때마다 호출해주는 함수 (BeginPlay에서 구독)
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// ASC가 MoveSpeed 값이 바뀔 때마다 호출 → CharacterMovement 속도에 반영
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	// DefaultAttributesEffect를 자기 자신에게 1번 적용해서 초기 스탯을 세팅
	void InitializeAttributes();

	// 사망 로직 (C++). virtual이라 자식 클래스가 동작을 추가할 수 있다
	virtual void Die();

	// 사망 연출 (BP): 이펙트/사운드/래그돌 등은 BP 이벤트 그래프에서 구현
	// BlueprintImplementableEvent : C++에는 선언만 있고 몸체는 BP에서 만든다 (C++에서 호출 가능)
	// Category                    : "Character" 그룹으로 표시
	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void OnDeath();

	// GAS 본체
	// VisibleAnywhere   : 에디터 디테일 패널에서 보이기만 함 (컴포넌트 자체를 다른 걸로 교체 불가)
	// BlueprintReadOnly : 블루프린트에서 읽기(Get)만 가능, 대입(Set) 불가
	// Category          : 디테일 패널/블루프린트에서 "GAS" 그룹으로 묶어서 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 체력 등 스탯 묶음 (지정자 의미는 위와 동일)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UShooterAttributeSet> AttributeSet;

	// 초기 스탯 GE (BP에서 GE_InitStats_Player / GE_InitStats_Enemy 지정).
	// Instant + Override 모디파이어로 체력/공격력/속도 등을 이 캐릭터 고유 값으로 덮어쓴다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "GAS" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

private:
	// 초기 스탯을 이미 적용했는지 (재빙의 시 체력이 다시 가득 차는 것 방지)
	bool bAttributesInitialized = false;
};
