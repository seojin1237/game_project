#pragma once

#include "CoreMinimal.h"
#include "Character/ShooterCharacterBase.h"
#include "ActiveGameplayEffectHandle.h"
#include "ShooterCharacter.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AShooterWeapon;
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

	// 지금 들고 있는 무기 (없으면 nullptr). 사격 어빌리티가 무기 데이터를 읽을 때 사용
	AShooterWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	// 새 무기를 스폰해서 손에 들기. 들고 있던 무기는 삭제 (나중에 무기 줍기/교체에 사용)
	// BlueprintCallable : BP에서 호출 가능한 노드로 노출
	// Category          : "Weapon" 그룹으로 표시
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(TSubclassOf<AShooterWeapon> WeaponClass);

protected:
	// 캐릭터가 사라질 때 들고 있던 무기도 같이 정리
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// 부모의 ASC 초기화 + 어빌리티 등록 + 기본 무기 장착
	virtual void PossessedBy(AController* NewController) override;

	// 컨트롤러가 바뀌었을 때 호출 → 여기서 Input Mapping Context를 등록한다
	virtual void NotifyControllerChanged() override;

	// 입력 컴포넌트가 준비되면 호출 → 여기서 Input Action과 함수를 연결(Bind)한다
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 입력 처리 함수들 (Input Action이 발동되면 호출됨)
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// 사격 버튼을 누른 순간 (단발/연사 무기 모두 1발)
	void FireStarted();
	// 사격 버튼을 누르고 있는 동안 매 프레임 (연사 무기만 계속 발사)
	void FireHeld();
	// 실제로 사격 어빌리티 실행을 시도하는 공통 함수
	void TryFire();

	// ===== 무기 =====

	// 무기를 붙일 위치. 지금은 캡슐 오른쪽 앞 (BP 뷰포트에서 위치 조정)
	// 나중에 스켈레탈 메시를 넣으면 이 컴포넌트의 부모를 메시의 손 소켓으로 바꾸면 된다
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(위치, 부모 소켓)은 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Weapon" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> WeaponAttachPoint;

	// 게임 시작 시 들고 시작할 무기 (BP에서 BP_Weapon_Rifle 지정)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AShooterWeapon> DefaultWeaponClass;

	// 지금 들고 있는 무기 인스턴스
	// VisibleInstanceOnly : 플레이 중 배치된 캐릭터의 디테일 패널에서만 보임 (클래스 기본값에는 없음)
	// Transient           : 저장하지 않음 (게임 중에만 존재하는 값)
	// Category            : "Weapon" 그룹으로 표시
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Weapon")
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	// 지금 무기의 EquipEffect가 적용된 "영수증". 무기를 바꿀 때 이걸로 정확히 그 효과만 제거한다
	// (C#에서 이벤트 구독 해제용으로 핸들러를 기억해 두는 것과 비슷)
	FActiveGameplayEffectHandle WeaponEffectHandle;

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
