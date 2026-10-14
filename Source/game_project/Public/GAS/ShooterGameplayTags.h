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
}
