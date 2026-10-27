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

	// WeaponSlots의 SlotIndex번째 무기로 교체 (0부터 시작). 이미 그 무기를 들고 있거나 번호가 없으면 무시
	// BlueprintCallable : BP에서 호출 가능한 노드로 노출 (나중에 UI 버튼 등에서 사용)
	// Category          : "Weapon" 그룹으로 표시
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipSlot(int32 SlotIndex);

protected:
	// 모든 컴포넌트가 만들어진 직후 호출 → 무기 부착 지점을 메시의 손 소켓에 붙인다
	// (생성자에서는 BP에서 바꾼 소켓 이름이 아직 반영 전이라 여기서 한다)
	virtual void PostInitializeComponents() override;

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

	// 무기를 붙일 위치. 게임 시작 시 캐릭터 메시의 WeaponSocketName 소켓(손)에 붙는다
	// 이 컴포넌트의 위치/회전 = "손 소켓 기준으로 총을 얼마나 옮기고 돌릴지" (BP에서 미세 조정)
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(위치, 회전)은 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Weapon" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> WeaponAttachPoint;

	// 무기를 붙일 메시 소켓 이름 (UE5 마네킹: HandGrip_R = 오른손 총 손잡이 위치)
	// 메시에 이 소켓이 없으면 메시 원점에 붙는다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Weapon" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("HandGrip_R");

	// 들고 다니는 무기 목록. [0] = 1번 키, [1] = 2번 키 ... 게임 시작 시 [0]을 든다
	// (BP에서 BP_Weapon_Rifle, BP_Weapon_SMG, BP_Weapon_Pistol 순서로 지정)
	// TArray = 크기가 변하는 배열 (C#의 List<T>)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<AShooterWeapon>> WeaponSlots;

	// 지금 들고 있는 슬롯 번호 (INDEX_NONE = -1 = 아직 아무것도 안 듦)
	// VisibleInstanceOnly : 플레이 중 배치된 캐릭터의 디테일 패널에서만 보임
	// Transient           : 저장하지 않음 (게임 중에만 존재하는 값)
	// Category            : "Weapon" 그룹으로 표시
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Weapon")
	int32 CurrentSlotIndex = INDEX_NONE;

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

	// 무기 슬롯 교체 (1/2/3 키, bool). [0]을 누르면 WeaponSlots[0]으로 교체
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TObjectPtr<UInputAction>> EquipSlotActions;

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
