#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterHealthBarWidget.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
struct FOnAttributeChangeData;

/**
 * 적 머리 위에 띄우는 작은 체력 바 (WBP_EnemyHealthBar의 C++ 부모).
 * 플레이어 HUD(UShooterHUDWidget)와 같은 방식으로 ASC의 Health/MaxHealth 변화를 구독해서 바를 갱신한다.
 * HUD와 달리 웨이브 정보, 사망 텍스트 같은 것은 없는 "가벼운" 버전.
 *
 * Abstract : 디자인 없이 단독으로 쓰지 않음 (반드시 WBP를 만들어 사용)
 */
UCLASS(Abstract)
class GAME_PROJECT_API UShooterHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 표시할 대상(적)의 ASC를 연결. 다시 호출하면 이전 연결은 끊고 새로 연결
	void BindToAbilitySystem(UAbilitySystemComponent* InASC);

protected:
	// 위젯이 사라질 때(적이 삭제될 때 등) 구독 해제
	virtual void NativeDestruct() override;

	// ASC가 Health/MaxHealth 변화를 알려줄 때 호출
	void HandleHealthChanged(const FOnAttributeChangeData& Data);

	// ASC에서 현재 체력을 읽어 바에 반영
	void RefreshHealth();

	// 체력 바. WBP에 "HealthBar"라는 이름의 Progress Bar가 반드시 있어야 한다
	// BindWidget : WBP 디자이너의 같은 이름 위젯과 자동 연결 (필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

private:
	// 구독 중인 ASC (Weak: 적이 먼저 삭제돼도 죽은 포인터를 잡고 있지 않도록)
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	// 구독 해제용 번호표 (C#에서 event -= 할 때 핸들러를 기억하는 것과 같은 역할)
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

	// 기존 구독 해제
	void Unbind();
};
