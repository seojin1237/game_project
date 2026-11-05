#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterDashAfterimage.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPoseableMeshComponent;
class USkeletalMeshComponent;

/**
 * 대시 잔상 하나. 스폰된 순간의 캐릭터 자세를 복사해 멈춰 있다가, 서서히 투명해진 뒤 사라진다.
 *
 * BP(BP_DashAfterimage)에서 지정할 것:
 *  - Ghost Material : 잔상의 겉모습 (M_DashAfterimage). "Fade" 스칼라 파라미터로 투명해진다
 *  - Fade Duration  : 사라지는 데 걸리는 시간
 */
UCLASS()
class GAME_PROJECT_API AShooterDashAfterimage : public AActor
{
	GENERATED_BODY()

public:
	AShooterDashAfterimage();

	// SourceMesh의 메시와 지금 자세를 복사하고, 사라지기 시작한다. 스폰 직후 호출할 것
	void CopyPose(USkeletalMeshComponent* SourceMesh);

protected:
	// 매 프레임 호출 (Unity의 Update) → 남은 수명에 맞춰 잔상을 투명하게 만든다
	virtual void Tick(float DeltaSeconds) override;

	// 자세를 직접 지정할 수 있는 메시. 애니메이션 없이 복사한 자세 그대로 멈춰 있는다
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값은 수정 가능, 컴포넌트 교체는 불가
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Afterimage" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Afterimage")
	TObjectPtr<UPoseableMeshComponent> PoseableMesh;

	// 잔상에 입힐 머티리얼. "Fade" 스칼라 파라미터가 있어야 투명해진다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Afterimage" 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Afterimage")
	TObjectPtr<UMaterialInterface> GhostMaterial;

	// 다 사라지는 데 걸리는 시간(초)
	// EditDefaultsOnly, Category : 위와 동일
	// meta=(ClampMin)            : 0으로 나누기와 수명 0(영원히 안 사라짐)을 막는다
	UPROPERTY(EditDefaultsOnly, Category = "Afterimage", meta = (ClampMin = "0.05"))
	float FadeDuration = 0.3f;

private:
	// 실행 중에 값을 바꿀 수 있는 머티리얼 복사본 (Unity의 renderer.material로 얻는 인스턴스와 비슷)
	// UPROPERTY() : 가비지 컬렉터가 추적하도록 (UObject 포인터 멤버엔 필수)
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> GhostMaterialInstance;
};
