#pragma once

#include "NativeGameplayTags.h"

/**
 * C++에서 쓰는 GameplayTag 모음.
 * 여기서 선언하면 에디터의 태그 목록에도 자동으로 등록된다 (Project Settings에서 따로 추가할 필요 없음).
 */
namespace ShooterTags
{
	// 죽은 상태. 이 태그가 있으면 사망 처리가 끝난 것으로 본다
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);

	// 질주 중. 질주 GE(GE_Sprint)가 적용돼 있는 동안 자동으로 붙는다 (GE의 Granted Tags)
	// GA_Fire의 Activation Blocked Tags에 있어서 질주 중에는 못 쏜다. 점프해도 Shift를 누르고 있으면 그대로 막힌다
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Sprinting);
	// 점프/낙하로 공중에 있음. 캐릭터 이동 모드가 바뀔 때 붙이고 뗀다
	// GA_Dash의 Activation Blocked Tags에 있어서 공중에서는 대시할 수 없다 (사격은 막지 않음)
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_InAir);
	// 무기 교체 애니메이션 재생 중. GA_Fire의 Activation Blocked Tags에 있어서 교체 중에는 못 쏜다
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Equipping);
	// 재장전 중. 재장전 어빌리티(GA_Reload)가 실행되는 동안 자동으로 붙는다 (Activation Owned Tags)
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Reloading);
	// 조준 중. 조준 GE(GE_Aim)가 적용돼 있는 동안 자동으로 붙는다 (GE의 Granted Tags)
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Aiming);
	// 대시 중. 대시 어빌리티(GA_Dash)가 실행되는 동안 자동으로 붙는다 (Activation Owned Tags)
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dashing);
	// 무적. 이 태그가 있는 대상에게는 공격이 들어가지 않는다 (GA_Dash의 Activation Owned Tags로 대시 동안 붙는다)
	GAME_PROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invincible);
}
