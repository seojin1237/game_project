#pragma once

#include "CoreMinimal.h"
#include "Character/ShooterCharacterBase.h"
#include "ShooterCharacter.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 플레이어 캐릭터.
 * ASC/AttributeSet/사망 처리는 부모(AShooterCharacterBase)가 담당하고,
 * 여기서는 카메라, 입력, 사격 어빌리티 등 플레이어 전용 기능만 다룬다.
 */
UCLASS()
class GAME_PROJECT_API AShooterCharacter : public AShooterCharacterBase
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	// [테스트용] 콘솔(~ 키)에 DebugDamage 입력 시 DebugDamageEffect를 자기 자신에게 적용
	// Exec : 콘솔 명령어로 호출 가능 (PlayerController, 빙의된 Pawn, GameMode 등에서만 동작)
	UFUNCTION(Exec)
	void DebugDamage();

protected:
	// 부모의 ASC 초기화 + 어빌리티 등록
	virtual void PossessedBy(AController* NewController) override;

	// 컨트롤러가 바뀌었을 때 호출 → 여기서 Input Mapping Context를 등록한다
	virtual void NotifyControllerChanged() override;

	// 입력 컴포넌트가 준비되면 호출 → 여기서 Input Action과 함수를 연결(Bind)한다
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 입력 처리 함수들 (Input Action이 발동되면 호출됨)
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Fire();

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

	// 사격 (마우스 왼쪽, bool)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	// ===== GAS (ASC/AttributeSet은 부모에 있음) =====

	// 사격 어빌리티 클래스 (BP에서 GA_Fire 지정). 게임 시작 시 ASC에 등록(Grant)된다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "GAS|Abilities" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> FireAbility;

	// [테스트용] DebugDamage에서 적용할 GameplayEffect 클래스 (BP에서 GE_Damage 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능 (레벨에 배치된 개별 인스턴스에선 수정 불가)
	// Category         : "GAS|Debug" 하위 그룹으로 표시 (| 는 그룹 안의 그룹)
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Debug")
	TSubclassOf<UGameplayEffect> DebugDamageEffect;
};
