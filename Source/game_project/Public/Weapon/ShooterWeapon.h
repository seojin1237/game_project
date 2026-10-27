#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UGameplayEffect;
class UAnimMontage;
class UNiagaraSystem;

/**
 * 총 1자루 (BP_Weapon_Rifle 같은 무기 BP의 C++ 부모).
 *
 * 역할 나누기:
 * - 무기(이 클래스): "어떤 총인가"에 대한 데이터 + 겉모습(메시, 총구 위치)
 *     → 데미지 Effect, 사거리, 연사 여부
 * - 사격 어빌리티(UShooterFireAbility): "어떻게 쏘는가"에 대한 로직 (트레이스, 데미지 적용)
 *     → 어빌리티는 하나, 무기는 여러 개. 어빌리티가 지금 든 무기의 데이터를 읽어서 쏜다
 *   (Unity로 치면 Gun 프리팹(ScriptableObject 데이터 포함)과 Shoot() 로직을 분리한 것)
 *
 * 무기는 캐릭터가 스폰해서 손(WeaponAttachPoint)에 붙인다. 실제 메시 에셋이 생기면 BP에서 메시만 바꾸면 된다.
 */
UCLASS(Abstract)
class GAME_PROJECT_API AShooterWeapon : public AActor
{
	GENERATED_BODY()

public:
	AShooterWeapon();

	// 총알이 나가는 위치 (총구). 사격 어빌리티가 트레이스 시작점으로 사용
	FVector GetMuzzleLocation() const;

	// 왼손이 잡을 위치 (월드 좌표). 애니메이션의 왼손 IK 목표로 사용
	FVector GetLeftHandGripLocation() const;

	TSubclassOf<UGameplayEffect> GetDamageEffect() const { return DamageEffect; }
	float GetRange() const { return Range; }
	bool IsAutomatic() const { return bAutomatic; }
	TSubclassOf<UGameplayEffect> GetEquipEffect() const { return EquipEffect; }
	UAnimMontage* GetFireMontage() const { return FireMontage; }
	UAnimMontage* GetEquipMontage() const { return EquipMontage; }

	// 총구 화염 이펙트를 총구 위치에 1번 재생 (MuzzleFlashEffect가 비어 있으면 아무것도 안 함)
	void PlayMuzzleFlash() const;

protected:
	// 무기의 기준점(루트). 손에 붙일 때 이 컴포넌트가 부착 지점에 딱 맞춰진다
	// 메시를 루트로 두지 않는 이유: 부착 시 루트의 회전은 부착 지점 회전으로 덮어써지므로,
	// 메시 에셋마다 다른 방향(Lyra 총은 +Y가 앞)을 맞추려면 메시가 루트 아래 자식이어야 회전을 줄 수 있다
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값은 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Weapon" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> WeaponRoot;

	// 총 모양. BP에서 메시 지정 + 회전/위치를 맞춘다 (+X가 총구 방향이 되도록)
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(메시, 크기 등)은 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Weapon" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// 총구 위치 표시용 빈 컴포넌트. BP 뷰포트에서 총 끝으로 옮겨 두면 된다 (지정자 의미는 위와 동일)
	// USceneComponent = 위치만 있는 빈 컴포넌트 (Unity의 빈 GameObject 자식과 같음)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> Muzzle;

	// 왼손이 잡을 위치 표시용 빈 컴포넌트 (총열 아래 손잡이 부분). BP 뷰포트에서 위치 조정 (지정자 의미는 위와 동일)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> LeftHandGrip;

	// 맞은 대상에게 적용할 데미지 Effect (BP에서 GE_Damage 지정)
	// 무기마다 다른 GE를 넣으면 "샷건은 넉백", "저격총은 출혈" 같은 차이를 줄 수 있다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon|Fire" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 사거리 (cm) (지정자 의미는 위와 동일)
	// meta=(ClampMin) : 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire", meta = (ClampMin = "0"))
	float Range = 5000.f;

	// true = 누르고 있으면 계속 발사(연사), false = 클릭할 때마다 1발(단발) (지정자 의미는 위와 동일)
	// 실제 발사 간격은 여전히 FireInterval 속성 + 쿨타임 GE가 정한다
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	bool bAutomatic = true;

	// 쏠 때마다 캐릭터가 재생할 사격 애니메이션 (BP에서 AM_Rifle_Fire 등 지정, 비우면 재생 안 함)
	// Montage = 코드에서 원하는 순간에 재생할 수 있는 애니메이션 에셋 (ABP의 Slot 노드를 통해 섞여 들어감)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon|Fire" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire")
	TObjectPtr<UAnimMontage> FireMontage;

	// 이 무기로 바꿔 들 때 재생할 애니메이션 (BP에서 AM_Rifle_Equip 등 지정, 비우면 재생 안 함)
	// 재생되는 동안은 State.Equipping 태그가 붙어서 사격이 막힌다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon|Equip" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Equip")
	TObjectPtr<UAnimMontage> EquipMontage;

	// 쏠 때마다 총구에서 재생할 Niagara 이펙트 (BP에서 NS_MuzzleFlash 등 지정, 비우면 재생 안 함)
	// Niagara System = 언리얼의 파티클 이펙트 에셋 (Unity의 Particle System / VFX Graph)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon|Effects" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
	TObjectPtr<UNiagaraSystem> MuzzleFlashEffect;

	// 이 무기를 들고 있는 동안 주인에게 걸리는 스탯 GE (Infinite로 만들 것)
	// 예: 권총 = AttackPower ×1.5 / 기관총 = FireInterval ×0.5, AttackPower ×0.7
	// 장착하면 적용, 다른 무기로 바꾸면 제거된다. 비워두면 스탯 변화 없음
	// (스탯을 C++ 변수로 두지 않고 GE로 만드는 이유: 업그레이드 GE와 같은 방식으로 자연스럽게 합쳐지고, BP에서 수치 조절 가능)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon|Stats" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Stats")
	TSubclassOf<UGameplayEffect> EquipEffect;
};
