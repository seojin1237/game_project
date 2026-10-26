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
	// 표시할 데미지 값 설정 (소수점 없이 표시)
	void SetDamage(float Damage);

protected:
	// [BP 확장용] 숫자가 정해졌을 때 호출. 크기 튀기기 애니메이션, 큰 데미지 색 변경 등 연출용
	// BlueprintImplementableEvent : C++은 선언만, 몸체는 BP 이벤트 그래프에서 구현 (구현 안 하면 아무 일 없음)
	// Category                    : "DamageNumber" 그룹으로 표시
	UFUNCTION(BlueprintImplementableEvent, Category = "DamageNumber")
	void OnDamageSet(float Damage);

	// 숫자 텍스트. WBP에 "DamageText"라는 이름의 Text가 반드시 있어야 한다
	// BindWidget : WBP 디자이너의 같은 이름 위젯과 자동 연결 (필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;
};
