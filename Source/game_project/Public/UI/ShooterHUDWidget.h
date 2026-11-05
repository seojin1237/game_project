#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "ShooterHUDWidget.generated.h"

class UAbilitySystemComponent;
class UImage;
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
	// 위젯이 만들어져 화면에 붙을 때 호출 (Unity의 OnEnable/Start와 비슷) → GameMode 웨이브 이벤트 구독
	virtual void NativeConstruct() override;

	// 위젯이 화면에서 제거될 때 호출 (Unity의 OnDestroy와 비슷) → 구독 해제
	virtual void NativeDestruct() override;

	// 매 프레임 호출 (Unity의 Update) → 피격 때 붉어진 화면을 서서히 되돌린다
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// GameMode가 웨이브 상태 변화를 알려줄 때 호출
	// UFUNCTION() : 다이내믹 델리게이트(AddDynamic)에 연결하려면 리플렉션에 등록된 함수여야 해서 필요
	UFUNCTION()
	void HandleWaveStateChanged(int32 WaveNumber, int32 EnemiesRemaining);

	// ASC가 Health/MaxHealth 변화를 알려줄 때 호출
	void HandleHealthAttributeChanged(const FOnAttributeChangeData& Data);

	// ASC가 State.Dead 태그 개수 변화를 알려줄 때 호출 (0 → 1이면 방금 죽은 것)
	void HandleDeadTagChanged(const FGameplayTag Tag, int32 NewCount);

	// ASC에서 현재 체력 값을 읽어 위젯에 반영
	void RefreshHealth();

	// ASC가 Ammo/MaxAmmo 변화를 알려줄 때 호출
	void HandleAmmoAttributeChanged(const FOnAttributeChangeData& Data);

	// ASC에서 현재 탄약 값을 읽어 위젯에 반영
	void RefreshAmmo();

	// 대시 쿨타임이 얼마나 지났는지 읽어 바에 반영 (쿨타임 이벤트가 없어 매 프레임 호출)
	void RefreshDashCooldown();

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

	// "WAVE 2  |  ENEMIES 5" 같은 웨이브 정보. 없어도 된다
	// BindWidgetOptional : 위와 동일 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveText;

	// "24 / 30" 같은 탄약 표시. 없어도 된다
	// BindWidgetOptional : 위와 동일 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AmmoText;

	// 피격 시 화면 가장자리를 붉게 만드는 전체 화면 이미지. 없어도 된다
	// BindWidgetOptional : 위와 동일 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageOverlay;

	// 대시 쿨타임 바. 가득 차 있으면 대시 가능. 없어도 된다
	// BindWidgetOptional : 위와 동일 (선택)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> DashCooldownBar;

	// 맞은 순간 붉은 화면의 진하기 (0 = 안 보임, 1 = 가장 진함)
	// EditDefaultsOnly : WBP_HUD의 Class Defaults에서만 수정 가능 (연출 수치라 BP에서 튜닝)
	// Category         : "HUD|Damage" 하위 그룹으로 표시
	// meta=(ClampMin, ClampMax) : 0 ~ 1 범위로 제한
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Damage", meta = (ClampMin = "0", ClampMax = "1"))
	float DamageFlashOpacity = 0.8f;

	// 붉은 화면이 완전히 사라지는 데 걸리는 시간 (초)
	// EditDefaultsOnly, Category : 위와 동일
	// meta=(ClampMin)            : 0으로 나누기 방지
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Damage", meta = (ClampMin = "0.05"))
	float DamageFlashDuration = 0.4f;

private:
	// 지금 붉은 화면의 진하기. 맞으면 DamageFlashOpacity가 되고 매 프레임 0을 향해 줄어든다
	float CurrentDamageFlash = 0.f;

	// 구독 중인 ASC. Weak 포인터 = 대상이 삭제되면 자동으로 nullptr이 되는 포인터
	// (캐릭터가 먼저 사라져도 위젯이 "죽은 포인터"를 잡고 있지 않도록. C#의 WeakReference와 비슷)
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	// 구독 해제할 때 필요한 "구독 번호표"들 (C#에서 event -= 할 때 핸들러를 기억해두는 것과 같은 역할)
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle DeadTagHandle;
	FDelegateHandle AmmoChangedHandle;
	FDelegateHandle MaxAmmoChangedHandle;

	// 기존 구독을 모두 해제
	void UnbindFromAbilitySystem();
};
