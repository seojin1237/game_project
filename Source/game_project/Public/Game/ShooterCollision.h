#pragma once

#include "Engine/EngineTypes.h"

/**
 * 프로젝트 전용 충돌 채널 이름 모음.
 *
 * 언리얼은 사용자 정의 채널을 ECC_GameTraceChannel1~18 같은 "번호"로만 제공한다.
 * 번호로 쓰면 무슨 채널인지 알기 어려우니, 여기서 의미 있는 이름을 붙여 쓴다.
 * 실제 채널 정의(이름, 기본 반응)는 Config/DefaultEngine.ini의 [/Script/Engine.CollisionProfile]에 있다.
 * → 에디터의 Project Settings > Collision > Trace Channels 에도 "Weapon"으로 보인다.
 *
 * (Unity로 치면 LayerMask.NameToLayer("Weapon")의 결과를 상수로 박아둔 것과 비슷)
 */

// 총알 판정용 트레이스 채널. 기본 반응 Block → 벽/바닥/캐릭터 캡슐이 모두 총알을 막는다.
// (엔진 기본 "Pawn" 프리셋은 Visibility 채널을 무시하기 때문에, Visibility로 쏘면 캐릭터가 안 맞는다)
#define ECC_Weapon ECC_GameTraceChannel1
