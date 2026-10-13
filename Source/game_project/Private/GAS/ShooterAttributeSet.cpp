#include "GAS/ShooterAttributeSet.h"

UShooterAttributeSet::UShooterAttributeSet()
{
	// 임시 초기값. 나중에 초기화용 GameplayEffect(GE_InitStats)로 옮겨서 BP에서 조절할 예정
	InitHealth(100.f);
	InitMaxHealth(100.f);
}
