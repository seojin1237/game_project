#pragma once

#include "CoreMinimal.h"
#include "Character/ShooterCharacterBase.h"
#include "ShooterEnemy.generated.h"

class UGameplayAbility;
class UWidgetComponent;
class UAnimSequenceBase;

/**
 * 적 캐릭터. AI 컨트롤러(AShooterAIController)가 조종한다.
 * - 이동: AI 컨트롤러가 플레이어를 추적
 * - 공격: AI 컨트롤러가 거리를 보고 TryAttack()을 호출 → AttackAbility 실행
 */
UCLASS()
class GAME_PROJECT_API AShooterEnemy : public AShooterCharacterBase
{
	GENERATED_BODY()

public:
	AShooterEnemy();

	// Target을 바라보고 공격 시도. 쿨타임 중이거나 죽었으면 GAS가/코드가 알아서 거부한다
	// 반환값: 실제로 공격 어빌리티가 실행됐으면 true
	bool TryAttack(const AActor* Target);

	// AI가 "이 거리 안이면 공격 시도"하는 기준 거리 (몸 중심 사이 거리, cm)
	float GetAttackStartDistance() const { return AttackStartDistance; }

protected:
	// 게임 시작 시 호출 → 머리 위 체력 바 위젯을 이 적의 ASC에 연결
	virtual void BeginPlay() override;

	// 부모의 ASC 초기화 + 공격 어빌리티 등록
	virtual void PossessedBy(AController* NewController) override;

	// 머리 위 체력 바 (BP에서 Widget Class에 WBP_EnemyHealthBar 지정)
	// UWidgetComponent = 위젯을 3D 월드의 특정 위치에 붙여 그리는 컴포넌트 (Unity의 World Space Canvas와 비슷)
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(위젯 클래스, 위치, 크기)은 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "Enemy|UI" 하위 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	// 부모의 사망 처리 + 사망 애니메이션 + 일정 시간 뒤 삭제
	virtual void Die() override;

	// 사망 애니메이션이 끝나면 호출 → 메시를 물리에 맡겨 바닥까지 쓰러지게 한다 (래그돌)
	void StartRagdoll();

	// 부모의 사망 판정 + 체력이 줄었으면 피격 애니메이션 재생
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data) override;

	// 맞았을 때 상체에 재생할 애니메이션 후보. 이 중 하나를 무작위로 재생 (비우면 재생 안 함)
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Enemy|Animation" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Animation")
	TArray<TObjectPtr<UAnimSequenceBase>> HitReactAnimations;

	// 죽을 때 재생할 애니메이션 후보. 이 중 하나를 무작위로 재생 (비우면 재생 안 함, 지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Animation")
	TArray<TObjectPtr<UAnimSequenceBase>> DeathAnimations;

	// 공격 어빌리티 클래스 (BP에서 GA_EnemyAttack 지정). 빙의 시 ASC에 등록(Grant)된다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Enemy|Combat" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	TSubclassOf<UGameplayAbility> AttackAbility;

	// 플레이어와의 거리가 이 값 이하면 공격 시도 (cm). 공격 어빌리티의 AttackRange와 비슷하게 맞출 것
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "Enemy|Combat" 하위 그룹으로 표시
	// meta=(ClampMin)  : 0 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat", meta = (ClampMin = "0"))
	float AttackStartDistance = 200.f;

	// 사망 후 액터가 사라지기까지 걸리는 시간(초). 0이면 사라지지 않음 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Enemy", meta = (ClampMin = "0"))
	float DeathLifeSpan = 3.f;

	// 죽을 때 떨어뜨릴 아이템 (BP에서 BP_Pickup_Health 지정). 비우면 떨어뜨리지 않는다
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 지정 가능
	// Category         : "Enemy|Drop" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Drop")
	TSubclassOf<AActor> DropClass;

	// 아이템을 떨어뜨릴 확률 (0 = 안 떨어뜨림, 1 = 항상)
	// EditDefaultsOnly, Category : 위와 동일
	// meta=(ClampMin, ClampMax)  : 0 ~ 1 범위로 제한
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Drop", meta = (ClampMin = "0", ClampMax = "1"))
	float DropChance = 0.25f;
};
