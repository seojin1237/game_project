#include "UI/ShooterDamageNumberWidget.h"
#include "Components/TextBlock.h"

void UShooterDamageNumberWidget::SetDamage(float Damage)
{
	if (DamageText)
	{
		// FText::AsNumber: 숫자를 화면용 텍스트로 (천 단위 구분 등 지역 설정도 반영)
		// FMath::RoundToInt: 반올림해서 정수로 (19.6 → 20)
		DamageText->SetText(FText::AsNumber(FMath::RoundToInt(Damage)));
	}

	OnDamageSet(Damage);
}
