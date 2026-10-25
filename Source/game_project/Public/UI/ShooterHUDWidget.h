#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ShooterHUDWidget.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
class UTextBlock;
struct FOnAttributeChangeData;

/**
 * 플레이어 HUD 위젯의 C++ 베이스 (WBP_HUD가 이 클래스를 부모로 사용).
 *
 * 역할 분담:
 *  - C++ (여기): ASC를 구독해서 체력 바/숫자를 갱신하고, 사망하면 사망 텍스트를 보이게 하는 "로직"
 *  - BP (WBP_HUD): 위젯 배치, 색, 폰트, 애니메이션 같은 "겉모습"
 *
 * meta=(BindWidget) 변수는 WBP 디자이너에 "같은 이름"의 위젯이 있으면 자동으로 연결된다.
 * (Unity에서 [SerializeField]로 UI 요소를 드래그해 연결하는 것을, 이름 일치로 자동 처리하는 방식)
 *
 * Abstract : 이 클래스 자체로는 화면에 못 띄움 (디자인이 없으니까). 반드시 WBP를 만들어 사용
 */
UCLASS(Abstract)
class GAME_PROJECT_API UShooterHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이 HUD가 표시할 대상(플레이어 캐릭터)의 ASC를 연결한다.
	// PlayerController가 캐릭터에 빙의할 때 호출. 다시 호출되면 이전 연결은 끊고 새로 연결
	void BindToAbilitySystem(UAbilitySystemComponent* InASC);

protected:
	// 위젯이 화면에서 제거될 때 호출 (Unity의 OnDestroy와 비슷) → 구독 해제
	virtual void NativeDestruct() override;

	// ASC가 Health/MaxHealth 변화를 알려줄 때 호출
	void HandleHealthAttributeChanged(const FOnAttributeChangeData& Data);

	// ASC가 State.Dead 태그 개수 변화를 알려줄 때 호출 (0 → 1이면 방금 죽은 것)
	void HandleDeadTagChanged(const FGameplayTag Tag, int32 NewCount);

	// ASC에서 현재 체력 값을 읽어 위젯에 반영
	void RefreshHealth();

	// [BP 확장용] 체력이 바뀔 때마다 호출. 피격 시 화면 깜빡임 같은 연출을 BP에서 붙일 수 있다
	// BlueprintImplementableEvent : C++은 선언만, 몸체는 BP 이벤트 그래프에서 구현 (구현 안 하면 아무 일 없음)
	// Category                    : "HUD" 그룹으로 표시
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnHealthUpdated(float CurrentHealth, float MaxHealth);

	// [BP 확장용] 사망했을 때 호출. 페이드 아웃, 사운드 등 연출용 (지정자 의미는 위와 동일)
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnOwnerDied();

	// 체력 바. WBP에 "HealthBar"라는 이름의 Progress Bar가 반드시 있어야 한다 (없으면 WBP 컴파일 에러)
	// BindWidget : WBP 디자이너의 같은 이름 위젯과 자동 연결 (필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	// "80 / 100" 같은 체력 숫자. 없어도 된다
	// BindWidgetOptional : 같은 이름 위젯이 있으면 연결, 없으면 nullptr (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	// 사망 시 보여줄 텍스트 ("YOU DIED" 등). 처음엔 숨겨두고 사망하면 표시. 없어도 된다
	// BindWidgetOptional : 위와 동일 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DeathText;

private:
	// 구독 중인 ASC. Weak 포인터 = 대상이 삭제되면 자동으로 nullptr이 되는 포인터
	// (캐릭터가 먼저 사라져도 위젯이 "죽은 포인터"를 잡고 있지 않도록. C#의 WeakReference와 비슷)
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	// 구독 해제할 때 필요한 "구독 번호표"들 (C#에서 event -= 할 때 핸들러를 기억해두는 것과 같은 역할)
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle DeadTagHandle;

	// 기존 구독을 모두 해제
	void UnbindFromAbilitySystem();
};
