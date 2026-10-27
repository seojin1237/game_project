#include "Weapon/ShooterWeapon.h"
#include "Components/StaticMeshComponent.h"

AShooterWeapon::AShooterWeapon()
{
	// 무기 자체는 매 프레임 할 일이 없으므로 Tick 끔 (성능)
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	SetRootComponent(WeaponRoot);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);

	// 총은 충돌 없음: 캐릭터 캡슐과 부딪히거나, 내 총알(트레이스)에 내 총이 맞으면 안 되니까
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(WeaponMesh);
	// 메시의 자식이라 메시를 회전시키면 총구도 같이 돈다 → 위치는 "메시 기준 좌표"로 지정
	// 기본 위치: 메시 앞쪽(+X) 50cm. 실제 위치는 BP 뷰포트에서 총 끝에 맞춘다
	Muzzle->SetRelativeLocation(FVector(50.f, 0.f, 0.f));

	// 총구처럼 메시의 자식 → 메시를 옮기거나 돌려도 같이 따라간다. 실제 위치는 BP에서 무기마다 지정
	LeftHandGrip = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandGrip"));
	LeftHandGrip->SetupAttachment(WeaponMesh);
}

FVector AShooterWeapon::GetLeftHandGripLocation() const
{
	return LeftHandGrip ? LeftHandGrip->GetComponentLocation() : GetActorLocation();
}

FVector AShooterWeapon::GetMuzzleLocation() const
{
	// GetComponentLocation: 컴포넌트의 월드 좌표 (Unity의 transform.position)
	return Muzzle ? Muzzle->GetComponentLocation() : GetActorLocation();
}
