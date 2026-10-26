#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterDamageNumberActor.generated.h"

class UWidgetComponent;

/**
 * 맞은 위치에 잠깐 떠서 위로 올라가며 사라지는 데미지 숫자 (BP_DamageNumber의 C++ 부모).
 * 데미지 숫자 GameplayCue(UShooterGCN_DamageNumber)가 스폰한다.
 *
 * 구조: 액터(위치/움직임/수명) + WidgetComponent(숫자 위젯 WBP_DamageNumber를 화면에 그림)
 */
UCLASS()
class GAME_PROJECT_API AShooterDamageNumberActor : public AActor
{
	GENERATED_BODY()

public:
	AShooterDamageNumberActor();

	// 표시할 데미지 설정. 스폰 직후 GameplayCue가 호출
	void SetDamage(float Damage);

	// 매 프레임 호출 (Unity의 Update) → 위로 이동 + 점점 투명하게
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	// 숫자 위젯을 담는 컴포넌트 (BP에서 Widget Class에 WBP_DamageNumber 지정)
	// VisibleAnywhere   : 디테일 패널에서 보이고 내부 값(위젯 클래스, 크기 등)은 수정 가능
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category          : "DamageNumber" 그룹으로 표시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageNumber")
	TObjectPtr<UWidgetComponent> NumberWidget;

	// 화면에 떠 있는 시간(초). 이 시간이 지나면 자동 삭제
	// EditDefaultsOnly : 블루프린트 클래스 기본값에서만 수정 가능
	// Category         : "DamageNumber" 그룹으로 표시
	// meta=(ClampMin)  : 0.1초 미만 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber", meta = (ClampMin = "0.1"))
	float Lifetime = 0.8f;

	// 위로 떠오르는 속도 (cm/초) (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	float RiseSpeed = 120.f;

	// 스폰 위치를 좌우로 살짝 흩뜨리는 범위 (cm). 연사할 때 숫자가 한 자리에 겹치지 않게 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber", meta = (ClampMin = "0"))
	float RandomOffset = 25.f;

private:
	// 생성된 뒤 흐른 시간 (투명도 계산용)
	float Age = 0.f;
};
