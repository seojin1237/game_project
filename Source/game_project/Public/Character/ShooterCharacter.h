#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ShooterCharacter.generated.h"

class UAbilitySystemComponent;
class UShooterAttributeSet;
class UGameplayEffect;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

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

	// 컨트롤러가 바뀌었을 때 호출 → 여기서 Input Mapping Context를 등록한다
	virtual void NotifyControllerChanged() override;

	// 입력 컴포넌트가 준비되면 호출 → 여기서 Input Action과 함수를 연결(Bind)한다
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 입력 처리 함수들 (Input Action이 발동되면 호출됨)
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// ===== 카메라 =====

	// 카메라를 캐릭터 뒤쪽에 일정 거리로 붙잡아 두는 "셀카봉". 벽에 닿으면 자동으로 줄어든다
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(길이, 오프셋 등)은 수정 가능, 컴포넌트 교체는 불가
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Camera" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	// 실제 화면을 찍는 카메라 (지정자 의미는 위와 동일)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// ===== 입력 (BP에서 IA_ / IMC_ 에셋 지정) =====

	// 키 ↔ Input Action 연결표
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Input" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 이동 (WASD, Vector2D) — 지정자 의미는 위와 동일
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// 시점 회전 (마우스, Vector2D)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	// 점프 (Space, bool)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	// ===== GAS =====

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
