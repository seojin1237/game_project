#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ShooterHealthPickup.generated.h"

class UGameplayEffect;
class USphereComponent;
class UStaticMeshComponent;

/**
 * 체력 회복 아이템. 적이 죽을 때 떨어뜨리고, 플레이어가 닿으면 HealEffect를 적용한 뒤 사라진다.
 *
 * BP(BP_Pickup_Health)에서 지정할 것:
 *  - MeshComponent의 Static Mesh / Material : 겉모습
 *  - Heal Effect                            : 회복량이 든 GE (GE_Pickup_Heal)
 *  - Pickup Cue Tag                         : 먹을 때 재생할 소리와 이펙트 (GameplayCue.Pickup.Health)
 *  - Bob Height / Bob Speed                 : 위아래로 떠다니는 폭과 속도
 *  - Initial Life Span                      : 안 먹으면 몇 초 뒤 사라질지
 */
UCLASS()
class GAME_PROJECT_API AShooterHealthPickup : public AActor
{
	GENERATED_BODY()

public:
	AShooterHealthPickup();

protected:
	// 매 프레임 호출 (Unity의 Update) → 메시를 위아래로 움직여 눈에 띄게 한다
	virtual void Tick(float DeltaSeconds) override;

	// 닿는 범위를 정하는 구체. 눈에 보이지 않고 이 컴포넌트가 충돌을 담당한다 (Unity의 Is Trigger 켠 SphereCollider)
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(반지름 등)은 수정 가능, 컴포넌트 교체는 불가
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Pickup" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USphereComponent> PickupSphere;

	// 눈에 보이는 메시. 어떤 모양을 쓸지는 BP에서 지정한다 (지정자 의미는 위와 동일)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 먹었을 때 플레이어에게 적용할 GE. 회복량은 C++이 아니라 이 GE에서 조절한다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Pickup" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TSubclassOf<UGameplayEffect> HealEffect;

	// 먹을 때 실행할 GameplayCue 태그. 어떤 소리와 이펙트가 나올지는 Cue 에셋(GC_Pickup_Health)에서 정한다 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	FGameplayTag PickupCueTag;

	// 위아래로 떠다니는 폭 (cm)
	// EditDefaultsOnly, Category : 위와 동일
	// meta=(ClampMin)            : 음수 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ClampMin = "0"))
	float BobHeight = 12.f;

	// 위아래로 떠다니는 속도. 클수록 빠르다 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ClampMin = "0"))
	float BobSpeed = 3.f;

private:
	// PickupSphere에 무언가 닿았을 때 호출
	// UFUNCTION() : OnComponentBeginOverlap은 다이내믹 델리게이트라서, 연결할 함수가 리플렉션에 등록돼 있어야 한다
	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
