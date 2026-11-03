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

	// 수평 이동 속도 (cm/s). Blend Space의 Speed 축에 연결
	// BlueprintReadOnly : ABP 그래프에서 읽기만 가능 (값은 C++이 계산)
	// Category          : "Shooter|Movement" 하위 그룹으로 표시
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Movement")
	float MoveSpeed = 0.f;

	// 몸 기준 이동 방향 (도). 0 = 앞, 90 = 오른쪽, ±180 = 뒤. Blend Space의 Direction 축에 연결 (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Movement")
	float MoveDirection = 0.f;

	// 위아래 속도 (cm/s). 위로 빠르게 움직이면 점프 상태로 들어간다 (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Movement")
	float VerticalSpeed = 0.f;

	// 이동 입력이 있고 실제로 움직이는 중인지. Idle ↔ Walk/Run 전환에 사용 (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Movement")
	bool bShouldMove = false;

	// 공중에 있는지. 점프/낙하 상태 전환과 발 IK 끄기에 사용 (지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Movement")
	bool bIsFalling = false;

	// 서 있는 동안 몸(캡슐)이 돈 만큼 메시를 반대로 돌리는 각도 (도). ABP의 Rotate Root Bone에 연결
	// → 몸은 카메라를 따라 돌아도 발은 제자리에 남는다
	// BlueprintReadOnly : ABP 그래프에서 읽기만 가능 (값은 C++이 계산)
	// Category          : "Shooter|TurnInPlace" 하위 그룹으로 표시
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|TurnInPlace")
	float RootYawOffset = 0.f;

	// 허리 뼈 하나에 더할 회전. RootYawOffset만큼 함께 돌아간 상체를 카메라 쪽으로 되돌린다
	// 허리 뼈 3개에 나눠 적용해야 한 곳만 꺾이지 않는다 (ABP의 Transform (Modify) Bone에 연결, 지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|TurnInPlace")
	FRotator SpineYawCorrection = FRotator::ZeroRotator;

	// 지금 재생할 하체 동작. 0 = 평소, 1 = 왼쪽으로 돌기, 2 = 오른쪽으로 돌기 (ABP의 Blend Poses by Int에 연결, 지정자 의미는 위와 동일)
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|TurnInPlace")
	int32 TurnIndex = 0;

	// 발을 제자리에 둘 수 있는 최대 각도 (도). 넘으면 발이 몸을 따라 끌려온다
	// EditDefaultsOnly : ABP의 Class Defaults에서 조절 (연출 수치라 BP에서 튜닝)
	// Category         : "Shooter|TurnInPlace" 하위 그룹으로 표시
	// meta=(ClampMin)  : 음수 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|TurnInPlace", meta = (ClampMin = "0"))
	float MaxRootYawOffset = 110.f;

	// 하체와 상체의 각도 차이가 이 값을 넘으면 발을 옮겨 돈다 (도, 지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|TurnInPlace", meta = (ClampMin = "0"))
	float TurnStartAngle = 70.f;

	// 도는 동작을 재생하는 시간 (초). 이 시간 동안 하체가 정면으로 돌아온다 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|TurnInPlace", meta = (ClampMin = "0.05"))
	float TurnDuration = 0.5f;

	// 움직이기 시작할 때 하체가 다시 정면으로 돌아오는 속도. 클수록 빠르다 (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|TurnInPlace", meta = (ClampMin = "0"))
	float RootYawReturnSpeed = 10.f;

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

	// true면 상체를 "총을 겨눈 자세"(MF_Rifle_Idle_ADS)로 바꾼다. 조준 중이거나 방금 쐈을 때
	// → 달리면서 쏴도 상체는 겨눈 채로, 다리만 달리기 (ABP의 Blend Poses by bool에 연결)
	// BlueprintReadOnly : ABP 그래프에서 읽기만 가능 (값은 C++이 계산)
	// Category          : "Shooter|Aim" 하위 그룹으로 표시
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Aim")
	bool bUpperBodyAim = false;

	// 마지막 사격 후 몇 초 동안 겨눈 자세를 유지할지. 짧으면 연사 사이사이에 팔이 내려갔다 올라온다
	// EditDefaultsOnly : ABP의 Class Defaults에서 조절 (연출 수치라 BP에서 튜닝)
	// Category         : "Shooter|Aim" 하위 그룹으로 표시
	// meta=(ClampMin)  : 음수 입력 방지
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Aim", meta = (ClampMin = "0"))
	float FireAimHoldTime = 0.5f;

private:
	// 이번 프레임에 몸이 돈 각도를 구하기 위한 이전 프레임 값
	float LastActorYaw = 0.f;
	bool bHasLastActorYaw = false;

	// 도는 동작의 남은 시간 (초)
	float TurnTimeLeft = 0.f;
	// 도는 동안 하체가 정면으로 돌아오는 속도 (도/초). 시작할 때 각도에 맞춰 정한다
	float TurnYawSpeed = 0.f;
};
