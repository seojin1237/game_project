#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UGameplayEffect;

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

	TSubclassOf<UGameplayEffect> GetDamageEffect() const { return DamageEffect; }
	float GetRange() const { return Range; }
	bool IsAutomatic() const { return bAutomatic; }
	TSubclassOf<UGameplayEffect> GetEquipEffect() const { return EquipEffect; }

protected:
	// 총 모양. 지금은 임시로 BP에서 큐브를 늘려서 사용
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(메시, 크기 등)은 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Weapon" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// 총구 위치 표시용 빈 컴포넌트. BP 뷰포트에서 총 끝으로 옮겨 두면 된다 (지정자 의미는 위와 동일)
	// USceneComponent = 위치만 있는 빈 컴포넌트 (Unity의 빈 GameObject 자식과 같음)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> Muzzle;

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

	// 이 무기를 들고 있는 동안 주인에게 걸리는 스탯 GE (Infinite로 만들 것)
	// 예: 권총 = AttackPower ×1.5 / 기관총 = FireInterval ×0.5, AttackPower ×0.7
	// 장착하면 적용, 다른 무기로 바꾸면 제거된다. 비워두면 스탯 변화 없음
	// (스탯을 C++ 변수로 두지 않고 GE로 만드는 이유: 업그레이드 GE와 같은 방식으로 자연스럽게 합쳐지고, BP에서 수치 조절 가능)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Weapon|Stats" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Stats")
	TSubclassOf<UGameplayEffect> EquipEffect;
};
