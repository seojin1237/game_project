#include "Character/ShooterEnemy.h"
#include "AI/ShooterAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterEnemy::AShooterEnemy()
{
	// 레벨에 배치했을 때뿐 아니라 나중에 웨이브로 스폰될 때도 AI 컨트롤러가 자동으로 빙의하게
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 자동 빙의할 때 어떤 AI 컨트롤러를 만들지 지정 (기본값은 아무것도 안 하는 AAIController)
	// StaticClass(): 클래스 자체를 가리키는 값 (C#의 typeof(AShooterAIController)와 비슷)
	AIControllerClass = AShooterAIController::StaticClass();

	// 적은 이동하는 방향으로 몸을 돌린다 (플레이어와 달리 카메라가 없으므로)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 플레이어(기본 600)보다 느리게 → 도망치면서 쏠 수 있게. 실제 튜닝은 BP_Enemy에서
	GetCharacterMovement()->MaxWalkSpeed = 350.f;
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
