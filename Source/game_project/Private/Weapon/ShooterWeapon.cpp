#include "Weapon/ShooterWeapon.h"
#include "Components/StaticMeshComponent.h"

AShooterWeapon::AShooterWeapon()
{
	// 무기 자체는 매 프레임 할 일이 없으므로 Tick 끔 (성능)
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	// 총은 충돌 없음: 캐릭터 캡슐과 부딪히거나, 내 총알(트레이스)에 내 총이 맞으면 안 되니까
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(WeaponMesh);
	// 기본 위치: 총 앞쪽(+X) 50cm. 실제 위치는 BP 뷰포트에서 총 끝에 맞춘다
	Muzzle->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
}

FVector AShooterWeapon::GetMuzzleLocation() const
{
	// GetComponentLocation: 컴포넌트의 월드 좌표 (Unity의 transform.position)
	return Muzzle ? Muzzle->GetComponentLocation() : GetActorLocation();
}
