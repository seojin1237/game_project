#include "Character/ShooterEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterEnemy::AShooterEnemy()
{
	// 레벨에 배치했을 때뿐 아니라 나중에 웨이브로 스폰될 때도 AI 컨트롤러가 자동으로 빙의하게
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 적은 이동하는 방향으로 몸을 돌린다 (플레이어와 달리 카메라가 없으므로)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AShooterEnemy::Die()
{
	// 부모 클래스의 공통 사망 처리 (태그, 충돌/이동 끄기, OnDeath 연출) 먼저 실행
	Super::Die();

	// 적만의 추가 처리: DeathLifeSpan초 뒤 자동 삭제 (Unity의 Destroy(gameObject, delay))
	if (DeathLifeSpan > 0.f)
	{
		SetLifeSpan(DeathLifeSpan);
	}
}
