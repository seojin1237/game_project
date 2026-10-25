#include "AI/ShooterAIController.h"
#include "Character/ShooterCharacterBase.h"
#include "Character/ShooterEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AShooterAIController::AShooterAIController()
{
	// 매 프레임 할 일은 없다. 추적은 타이머로 띄엄띄엄 처리 (성능)
	PrimaryActorTick.bCanEverTick = false;
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// ChaseUpdateInterval초마다 UpdateChase를 반복 호출하는 타이머 시작
	// SetTimer(번호표, 대상 객체, 호출할 함수, 간격, 반복 여부, 첫 호출까지 지연)
	// → Unity의 InvokeRepeating("UpdateChase", 0, interval)과 같은 역할
	GetWorldTimerManager().SetTimer(
		ChaseTimerHandle, this, &AShooterAIController::UpdateChase,
		ChaseUpdateInterval, /*bLoop=*/ true, /*FirstDelay=*/ 0.f);
}

void AShooterAIController::OnUnPossess()
{
	// 조종할 몸이 없어졌으니 타이머 정지 (안 멈추면 없는 캐릭터에 계속 명령을 보낸다)
	GetWorldTimerManager().ClearTimer(ChaseTimerHandle);

	Super::OnUnPossess();
}

void AShooterAIController::UpdateChase()
{
	// 1) 내가 조종하는 적이 죽었으면 추적 중단
	//    GetPawn(): 이 컨트롤러가 빙의한 캐릭터. Cast로 우리 베이스 클래스인지 확인
	const AShooterCharacterBase* MyCharacter = Cast<AShooterCharacterBase>(GetPawn());
	if (!MyCharacter || MyCharacter->IsDead())
	{
		StopMovement();
		GetWorldTimerManager().ClearTimer(ChaseTimerHandle);
		return;
	}

	// 2) 플레이어 캐릭터 찾기 (싱글플레이라 0번 플레이어 = 나)
	//    UGameplayStatics: 자주 쓰는 전역 유틸 함수 모음 (Unity의 GameObject.FindWithTag 같은 용도)
	//    const를 붙이지 않는 이유: MoveToActor가 일반(AActor*) 포인터를 받기 때문
	//    (const 포인터 = "읽기만 하겠다"는 약속이라, 수정 가능한 포인터 자리에 넘길 수 없다)
	AShooterCharacterBase* Player =
		Cast<AShooterCharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));

	// 3) 플레이어가 없거나 죽었으면 그 자리에 멈춤
	if (!Player || Player->IsDead())
	{
		StopMovement();
		return;
	}

	// 4) 공격 거리 안이면 공격 시도 (적 캐릭터일 때만)
	//    Dist: 몸 중심 사이 거리. FVector::Dist = 두 점 사이 거리 (Unity의 Vector3.Distance)
	if (AShooterEnemy* Enemy = Cast<AShooterEnemy>(GetPawn()))
	{
		const float Dist = FVector::Dist(Enemy->GetActorLocation(), Player->GetActorLocation());
		if (Dist <= Enemy->GetAttackStartDistance())
		{
			// 쿨타임 중이면 GAS가 거부하므로 여기선 신경 쓸 필요 없다 (플레이어 연사와 같은 구조)
			Enemy->TryAttack(Player);
		}
	}

	// 5) 플레이어에게 이동. NavMesh 위에서 장애물을 피해 길을 찾아간다
	//    (Unity의 NavMeshAgent.SetDestination(player.position)과 같은 역할)
	//    매번 다시 호출하는 이유: 플레이어가 계속 움직이므로 목표 위치를 갱신해야 해서
	//    공격 중에도 호출해서, 플레이어가 물러나면 바로 다시 따라가게 한다
	MoveToActor(Player, AcceptanceRadius);
}
