#include "UI/ShooterDamageNumberWidget.h"
#include "Components/TextBlock.h"

void UShooterDamageNumberWidget::SetDamage(float Damage, bool bCritical)
{
	if (DamageText)
	{
		// FText::AsNumber: 숫자를 화면용 텍스트로 (천 단위 구분 등 지역 설정도 반영)
		// FMath::RoundToInt: 반올림해서 정수로 (19.6 → 20)
		FText Text = FText::AsNumber(FMath::RoundToInt(Damage));

		if (bCritical)
		{
			// 숫자 뒤에 접미사 붙이기 ("40" → "40!")
			// FText::Format: C#의 string.Format과 같은 역할 ({0}, {1} 자리에 값을 끼워 넣음)
			Text = FText::Format(FText::FromString(TEXT("{0}{1}")), Text, FText::FromString(CriticalSuffix));

			DamageText->SetColorAndOpacity(CriticalColor);

			// 글자 크기 키우기. 렌더 스케일 대신 폰트 크기를 바꾸는 이유:
			// 위젯 컴포넌트가 "원래 크기(DesiredSize)"대로 그리므로, 폰트를 키워야 그릴 영역도 같이 커져서 잘리지 않는다
			FSlateFontInfo Font = DamageText->GetFont();
			Font.Size = FMath::RoundToInt(Font.Size * CriticalFontScale);
			DamageText->SetFont(Font);
		}

		DamageText->SetText(Text);
	}

	OnDamageSet(Damage, bCritical);
}
