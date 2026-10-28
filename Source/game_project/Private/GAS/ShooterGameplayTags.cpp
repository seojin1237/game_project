#include "GAS/ShooterGameplayTags.h"

namespace ShooterTags
{
	// (C++ 변수 이름, 실제 태그 문자열, 에디터에 표시될 설명)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dead, "State.Dead", "사망 상태");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Sprinting, "State.Sprinting", "질주 중 (GE_Sprint가 부여)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_InAir, "State.InAir", "공중에 떠 있음 (점프/낙하)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Equipping, "State.Equipping", "무기 교체 애니메이션 중");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Reloading, "State.Reloading", "재장전 중 (GA_Reload가 부여)");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Aiming, "State.Aiming", "조준 중 (GE_Aim이 부여)");
}
