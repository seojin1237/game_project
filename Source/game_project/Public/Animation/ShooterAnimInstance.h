#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 플레이어 애니메이션 BP(ABP_Rifle)의 C++ 부모.
 *
 * AnimInstance란? 메시 하나의 애니메이션을 매 프레임 계산하는 객체 (Unity의 Animator와 비슷).
 * ABP(애니메이션 블루프린트)는 이 클래스를 상속해서 AnimGraph(포즈 계산 그래프)를 그린다.
 *
 * 역할 나누기 (로직은 C++, 연출은 BP):
 * - C++(여기): "왼손을 어디에 붙여야 하는지" 계산 → 변수로 제공
 * - ABP: 그 변수를 Two Bone IK 노드에 연결해서 실제로 팔을 구부림
 */
UCLASS()
class GAME_PROJECT_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	// 매 프레임 애니메이션 계산 전에 호출 (Unity의 Update와 비슷, 게임 스레드에서 실행)
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 왼손이 붙을 목표 위치 (월드 좌표). 들고 있는 무기의 LeftHandGrip 위치
	// BlueprintReadOnly : ABP 그래프에서 읽기만 가능 (값은 C++이 계산)
	// Category          : "Shooter|IK" 하위 그룹으로 표시
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|IK")
	FVector LeftHandIKLocation = FVector::ZeroVector;

	// 왼손 IK를 얼마나 적용할지 (0 = 원래 애니메이션 그대로, 1 = 목표 위치에 완전히 고정)
	// 무기가 없거나 죽었으면 0 → 팔이 이상하게 꺾이지 않게 (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|IK")
	float LeftHandIKAlpha = 0.f;

	// 위아래 조준 각도 (도). 위를 보면 +, 아래를 보면 - (약 -90 ~ 90)
	// ABP의 Aim Offset 노드(AO_Rifle)에 연결해서 상체와 총이 바라보는 방향을 따라가게 한다
	// BlueprintReadOnly : ABP 그래프에서 읽기만 가능 (값은 C++이 계산)
	// Category          : "Shooter|Aim" 하위 그룹으로 표시
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Aim")
	float AimPitch = 0.f;
};
