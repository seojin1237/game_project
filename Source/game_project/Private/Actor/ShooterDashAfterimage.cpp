#include "Actor/ShooterDashAfterimage.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AShooterDashAfterimage::AShooterDashAfterimage()
{
	// 투명해지는 처리를 Tick에서 한다
	PrimaryActorTick.bCanEverTick = true;

	PoseableMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMesh"));
	SetRootComponent(PoseableMesh);

	// 잔상은 보이기만 한다. 총알이나 적이 부딪히면 안 되고, 잠깐 있다 사라지니 그림자도 필요 없다
	PoseableMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoseableMesh->SetCastShadow(false);
}

void AShooterDashAfterimage::CopyPose(USkeletalMeshComponent* SourceMesh)
{
	// 같은 메시를 지정한 뒤 뼈 자세를 복사한다
	PoseableMesh->SetSkinnedAssetAndUpdate(SourceMesh->GetSkinnedAsset());
	PoseableMesh->CopyPoseFromSkeletalComponent(SourceMesh);

	// 복사한 자세를 바로 반영한다. 다음 Tick까지 기다리면 한 프레임 동안 T자 자세로 보일 수 있다
	PoseableMesh->RefreshBoneTransforms();

	// 슬롯이 여러 개여도 잔상은 전부 같은 모습이라 복사본 하나를 같이 쓴다
	GhostMaterialInstance = UMaterialInstanceDynamic::Create(GhostMaterial, this);
	for (int32 SlotIndex = 0; SlotIndex < PoseableMesh->GetNumMaterials(); ++SlotIndex)
	{
		PoseableMesh->SetMaterial(SlotIndex, GhostMaterialInstance);
	}

	// SetLifeSpan: 이 시간(초) 뒤 자동으로 사라진다 (Unity의 Destroy(gameObject, 시간))
	SetLifeSpan(FadeDuration);
}

void AShooterDashAfterimage::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// GetLifeSpan: 남은 수명(초). 수명을 전체 시간으로 나누면 1에서 0으로 줄어드는 값이 된다
	if (GhostMaterialInstance)
	{
		GhostMaterialInstance->SetScalarParameterValue(TEXT("Fade"), GetLifeSpan() / FadeDuration);
	}
}
