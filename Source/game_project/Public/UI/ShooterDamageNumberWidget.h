#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterDamageNumberWidget.generated.h"

class UTextBlock;

/**
 * 떠오르는 데미지 숫자 1개 (WBP_DamageNumber의 C++ 부모).
 * 숫자를 표시하는 것만 담당하고, 위로 떠오르기/사라지기 움직임은 AShooterDamageNumberActor가 한다.
 *
 * Abstract : 디자인 없이 단독으로 쓰지 않음 (반드시 WBP를 만들어 사용)
 */
UCLASS(Abstract)
class GAME_PROJECT_API UShooterDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 표시할 데미지 값 설정 (소수점 없이 표시). bCritical이면 색/크기/접미사를 치명타용으로 바꾼다
	void SetDamage(float Damage, bool bCritical);

protected:
	// [BP 확장용] 숫자가 정해졌을 때 호출. 크기 튀기기 애니메이션, 치명타 전용 연출 등
	// BlueprintImplementableEvent : C++은 선언만, 몸체는 BP 이벤트 그래프에서 구현 (구현 안 하면 아무 일 없음)
	// Category                    : "DamageNumber" 그룹으로 표시
	UFUNCTION(BlueprintImplementableEvent, Category = "DamageNumber")
	void OnDamageSet(float Damage, bool bCritical);

	// 치명타 숫자 색 (평타 색은 WBP 디자이너에서 DamageText에 지정한 색 그대로)
	// EditDefaultsOnly : WBP 클래스 기본값(디테일 패널)에서만 수정 가능
	// Category         : "DamageNumber|Critical" 하위 그룹으로 표시
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber|Critical")
	FSlateColor CriticalColor = FSlateColor(FLinearColor(1.f, 0.15f, 0.05f));

	// 치명타일 때 글자 크기 배율 (1.5 = 1.5배) (지정자 의미는 위와 동일)
	// meta=(ClampMin) : 1 미만(치명타가 더 작아지는 것) 방지
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber|Critical", meta = (ClampMin = "1"))
	float CriticalFontScale = 1.5f;

	// 치명타 숫자 뒤에 붙일 글자 (예: "40!") (지정자 의미는 위와 동일)
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber|Critical")
	FString CriticalSuffix = TEXT("!");

	// 숫자 텍스트. WBP에 "DamageText"라는 이름의 Text가 반드시 있어야 한다
	// BindWidget : WBP 디자이너의 같은 이름 위젯과 자동 연결 (필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;
};
