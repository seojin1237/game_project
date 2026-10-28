#include "Character/ShooterCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Weapon/ShooterWeapon.h"
#include "GAS/ShooterGameplayTags.h"
#include "GAS/ShooterAttributeSet.h"
#include "TimerManager.h"

AShooterCharacter::AShooterCharacter()
{
	// ===== 회전 방식: 슈터 스타일 =====
	// 캐릭터 몸이 항상 카메라(컨트롤러)가 보는 방향으로 돈다 → 조준 방향 = 캐릭터 정면
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	// 이동 방향으로 몸을 돌리는 기능은 끔 (위 설정과 충돌하므로)
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// ===== 카메라 =====
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;                      // 캐릭터와 카메라 거리
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 60.f);      // 오른쪽 어깨 너머 시점
	CameraBoom->bUsePawnControlRotation = true;               // 마우스 회전을 따라 봉이 돈다

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 봉 끝에 부착
	FollowCamera->bUsePawnControlRotation = false;            // 회전은 봉이 담당

	// ===== 무기 부착 위치 =====
	// 일단 메시에 붙여 두고, 실제 손 소켓 부착은 PostInitializeComponents에서 한다
	WeaponAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponAttachPoint"));
	WeaponAttachPoint->SetupAttachment(GetMesh());

	// ASC/AttributeSet 생성은 부모(AShooterCharacterBase) 생성자에서 이미 처리됨
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 무기 부착 지점을 손 소켓으로 옮긴다
	// KeepRelativeTransform: BP에서 지정한 위치/회전 오프셋을 "소켓 기준"으로 그대로 유지
	if (WeaponAttachPoint && GetMesh())
	{
		WeaponAttachPoint->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
	}

	// BP에서 정한 평소 카메라 거리/시야각을 기억 (생성자 시점엔 BP 값이 아직 반영 전이라 여기서)
	DefaultArmLength = CameraBoom->TargetArmLength;
	DefaultFOV = FollowCamera->FieldOfView;
}

void AShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 조준 중이면 카메라를 당기고 시야각을 좁혀 확대, 아니면 평소 값으로. FInterpTo로 매 프레임 조금씩 따라가게 해서 부드럽게
	const bool bAiming = IsAiming() && !IsDead();
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, bAiming ? AimArmLength : DefaultArmLength, DeltaSeconds, AimInterpSpeed);
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, bAiming ? AimFOV : DefaultFOV, DeltaSeconds, AimInterpSpeed));
}

void AShooterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 무기는 별도 액터라 캐릭터가 사라져도 자동으로 안 지워진다 → 직접 정리
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AShooterCharacter::EquipWeapon(TSubclassOf<AShooterWeapon> WeaponClass)
{
	UWorld* World = GetWorld();
	if (!WeaponClass || !World)
	{
		return;
	}

	// 이전 무기의 스탯 GE 제거 (영수증(Handle)으로 정확히 그 효과만 지운다)
	if (AbilitySystemComponent && WeaponEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(WeaponEffectHandle);
		WeaponEffectHandle.Invalidate();
	}

	// 들고 있던 무기는 버린다 (지금은 삭제. 나중에 바닥에 떨어뜨리기 등으로 바꿀 수 있음)
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	// Owner/Instigator를 나로 지정 → 무기가 "누구 것인지" 알 수 있게
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = World->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), Params);
	if (CurrentWeapon)
	{
		// AttachToComponent: 다른 컴포넌트의 자식으로 붙이기 (Unity의 transform.SetParent)
		// SnapToTargetNotIncludingScale: 위치/회전은 부착 지점에 딱 맞추고, 크기는 무기 원래 크기 유지
		CurrentWeapon->AttachToComponent(WeaponAttachPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		// 새 무기의 스탯 GE를 나에게 적용 (Infinite GE라 제거할 때까지 유지)
		if (AbilitySystemComponent && CurrentWeapon->GetEquipEffect())
		{
			// Context에 "이 효과의 출처는 이 무기"라고 기록 (디버깅/확장용)
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			Context.AddSourceObject(CurrentWeapon);

			const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(CurrentWeapon->GetEquipEffect(), 1.f, Context);
			if (Spec.IsValid())
			{
				// 반환값(Handle)을 저장해 둬야 나중에 이 효과만 골라서 제거할 수 있다
				WeaponEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}

		// 새 무기의 탄창 크기를 MaxAmmo의 기본값(BaseValue)으로 넣고, 탄약을 가득 채운다
		// SetNumericAttributeBase: GE 없이 BaseValue를 직접 바꾸는 함수 (장비 교체처럼 "설정"할 때 사용)
		// 업그레이드 GE(MaxAmmo 배율)는 BaseValue 위에 따로 계산되므로, 어떤 무기로 바꿔도 배율이 유지된다
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->SetNumericAttributeBase(UShooterAttributeSet::GetMaxAmmoAttribute(), CurrentWeapon->GetMagazineSize());
			AbilitySystemComponent->SetNumericAttributeBase(UShooterAttributeSet::GetAmmoAttribute(),
				AbilitySystemComponent->GetNumericAttribute(UShooterAttributeSet::GetMaxAmmoAttribute()));
		}
	}
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	// 부모: ASC 초기화(InitAbilityActorInfo)
	Super::PossessedBy(NewController);

	// 어빌리티 등록(Grant): "이 캐릭터는 이 어빌리티를 사용할 수 있다"고 ASC에 알려준다.
	// 등록만 하는 것이고, 실행(Activate)은 입력이 들어왔을 때 따로 한다
	if (AbilitySystemComponent && FireAbility)
	{
		// Spec = 어빌리티 클래스 + 레벨(1)을 묶은 "등록 정보"
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(FireAbility, 1));
	}
	if (AbilitySystemComponent && ReloadAbility)
	{
		// GiveAbility의 반환값(번호표)을 저장 → 나중에 이 어빌리티만 골라서 취소할 수 있다
		ReloadAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(ReloadAbility, 1));
	}

	// 첫 번째 슬롯 무기 장착. BeginPlay가 아니라 여기서 하는 이유:
	// 무기 스탯 GE를 걸려면 ASC가 초기화(부모 PossessedBy의 InitAbilityActorInfo)된 뒤여야 하는데,
	// GameMode가 스폰한 플레이어는 BeginPlay → PossessedBy 순서로 호출되기 때문
	// (재빙의될 때 무기를 또 만들지 않도록 이미 들고 있으면 건너뜀)
	if (!CurrentWeapon)
	{
		EquipSlot(0);

		// 게임 시작은 "이미 총을 든 상태"로 본다 → 교체 애니메이션과 사격 금지(State.Equipping)를 바로 끝낸다
		// (안 그러면 시작하자마자 교체 시간만큼 쏘지도 장전하지도 못한다)
		StopAnimMontage();                                   // 인자 없이 부르면 지금 재생 중인 Montage를 멈춘다
		GetWorldTimerManager().ClearTimer(EquipTimerHandle); // 예약해 둔 EndEquip 타이머 취소
		EndEquip();
	}
}

void AShooterCharacter::EquipSlot(int32 SlotIndex)
{
	// IsValidIndex: 배열 범위 안의 번호인지 확인 (C#에서 index < list.Count 검사와 같음)
	// 같은 슬롯을 다시 누르면 무기를 새로 만들 필요가 없으니 무시, 죽은 뒤에도 무시
	if (!WeaponSlots.IsValidIndex(SlotIndex) || SlotIndex == CurrentSlotIndex || IsDead())
	{
		return;
	}

	// 재장전 중이었다면 취소 (탄창이 채워지기 전에 끊기므로, 바꾼 무기에 엉뚱하게 탄약이 채워지지 않는다)
	if (AbilitySystemComponent && ReloadAbilityHandle.IsValid())
	{
		AbilitySystemComponent->CancelAbilityHandle(ReloadAbilityHandle);
	}

	// 바꾸기 전에 지금 무기의 남은 탄약을 슬롯별로 저장 → 다시 꺼내 들면 그대로 (무기 교체로 공짜 재장전 방지)
	// Init(값, 개수): 배열을 개수만큼 같은 값으로 채운다. -1 = "아직 한 번도 안 든 무기"
	if (SlotAmmo.Num() != WeaponSlots.Num())
	{
		SlotAmmo.Init(-1.f, WeaponSlots.Num());
	}
	if (AbilitySystemComponent && SlotAmmo.IsValidIndex(CurrentSlotIndex))
	{
		SlotAmmo[CurrentSlotIndex] = AbilitySystemComponent->GetNumericAttribute(UShooterAttributeSet::GetAmmoAttribute());
	}

	// 실제 교체(이전 무기·스탯 GE 정리 → 새 무기 스폰·부착 → 새 스탯 GE·탄창 적용)는 EquipWeapon이 전부 처리
	EquipWeapon(WeaponSlots[SlotIndex]);

	// 스폰에 성공했을 때만 번호를 기억 (슬롯이 비어 있었으면 그대로)
	if (!CurrentWeapon)
	{
		return;
	}
	CurrentSlotIndex = SlotIndex;

	// 전에 들었던 무기면 저장해 둔 탄약으로 되돌린다 (처음이면 EquipWeapon이 채운 "가득" 그대로)
	if (AbilitySystemComponent && SlotAmmo[SlotIndex] >= 0.f)
	{
		AbilitySystemComponent->SetNumericAttributeBase(UShooterAttributeSet::GetAmmoAttribute(), SlotAmmo[SlotIndex]);
	}

	// 교체 애니메이션 재생. PlayAnimMontage는 재생 시간(초)을 돌려준다 (실패하면 0)
	// 사격 금지는 애니메이션 전체가 아니라 EquipLockRatio만큼만 (총이 올라온 뒤의 마무리 동작 동안은 쏠 수 있게)
	const float EquipDuration = CurrentWeapon->GetEquipMontage() ? PlayAnimMontage(CurrentWeapon->GetEquipMontage()) * EquipLockRatio : 0.f;
	if (AbilitySystemComponent && EquipDuration > 0.f)
	{
		// Loose Tag: GE 없이 코드에서 직접 붙였다 떼는 태그. 개수(Count)로 관리되며 1 이상이면 "있음"
		// AddLoose를 쓰지 않고 Count를 1로 "설정"하는 이유: 교체 중에 또 교체해도 태그가 2개로 쌓이지 않게
		AbilitySystemComponent->SetLooseGameplayTagCount(ShooterTags::State_Equipping, 1);

		// 애니메이션이 끝나는 시점에 EndEquip 호출. 교체 중에 다시 교체하면 같은 핸들로 타이머가 새로 시작된다
		GetWorldTimerManager().SetTimer(EquipTimerHandle, this, &AShooterCharacter::EndEquip, EquipDuration, false);
	}
}

void AShooterCharacter::EndEquip()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ShooterTags::State_Equipping, 0);
	}
}

void AShooterCharacter::StartSprint()
{
	// 이미 질주 중이거나, 조준 중이거나, 죽었거나, GE를 지정 안 했으면 무시
	if (!AbilitySystemComponent || !SprintEffect || SprintEffectHandle.IsValid() || IsAiming() || IsDead())
	{
		return;
	}

	// 무기 스탯 GE와 같은 방식: Spec을 만들어 나에게 적용하고, 영수증(Handle)을 저장
	// GE 안의 MoveSpeed 배율 → OnMoveSpeedChanged → MaxWalkSpeed 로 자동 반영되고,
	// GE의 Granted Tags(State.Sprinting) → 사격 어빌리티가 막힌다
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(SprintEffect, 1.f, AbilitySystemComponent->MakeEffectContext());
	if (Spec.IsValid())
	{
		SprintEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void AShooterCharacter::StopSprint()
{
	// GE를 제거하면 속도 배율과 State.Sprinting 태그가 같이 사라진다
	if (AbilitySystemComponent && SprintEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(SprintEffectHandle);
		SprintEffectHandle.Invalidate();
	}
}

void AShooterCharacter::StartAim()
{
	if (!AbilitySystemComponent || !AimEffect || AimEffectHandle.IsValid() || IsDead())
	{
		return;
	}

	// 조준과 질주는 동시에 하지 않는다 → 조준이 우선 (질주 중이었으면 끈다)
	StopSprint();

	// 질주와 같은 방식: GE의 MoveSpeed 배율로 느려지고, Granted Tags로 State.Aiming이 붙는다
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(AimEffect, 1.f, AbilitySystemComponent->MakeEffectContext());
	if (Spec.IsValid())
	{
		AimEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void AShooterCharacter::StopAim()
{
	if (AbilitySystemComponent && AimEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(AimEffectHandle);
		AimEffectHandle.Invalidate();
	}
}

void AShooterCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// IsFalling: 점프 중이거나 떨어지는 중 (Unity의 !isGrounded)
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ShooterTags::State_InAir, GetCharacterMovement()->IsFalling() ? 1 : 0);
	}
}

void AShooterCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// BP에서 IMC를 아직 지정 안 했으면 아무것도 안 함
	if (!DefaultMappingContext)
	{
		return;
	}

	// 플레이어가 조종할 때만 (AI 컨트롤러면 Cast 실패 → 무시)
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		// Enhanced Input 서브시스템은 LocalPlayer(이 PC 앞에 앉은 플레이어)마다 하나씩 있다
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// 우선순위 0으로 등록. 나중에 UI용 IMC 등을 더 높은 우선순위로 겹쳐 쓸 수 있다
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 프로젝트 기본 입력 컴포넌트가 Enhanced Input용인지 확인 후 사용
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Triggered : 키를 누르고 있는 동안 매 프레임
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);

		// Started : 누른 순간 1번 / Completed : 뗀 순간 1번
		// Jump/StopJumping은 부모(ACharacter)에 이미 있는 함수를 그대로 연결
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// 누른 순간 1발 + 누르고 있는 동안 매 프레임 시도(연사 무기만)
		// 실제 발사 간격은 쿨타임 GE(GE_Cooldown_Fire)가 걸러준다
		EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::FireStarted);
		EIC->BindAction(FireAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FireHeld);

		// 무기 교체 키: 키마다 함수를 따로 만들지 않고, 같은 EquipSlot에 "몇 번 슬롯인지"를 덧붙여 연결
		// BindAction 마지막 인자(i)는 호출될 때 함수에 그대로 전달된다 (C#으로 치면 () => EquipSlot(i) 람다)
		for (int32 i = 0; i < EquipSlotActions.Num(); ++i)
		{
			EIC->BindAction(EquipSlotActions[i], ETriggerEvent::Started, this, &AShooterCharacter::EquipSlot, i);
		}

		// 재장전: 누른 순간 1번
		EIC->BindAction(ReloadAction, ETriggerEvent::Started, this, &AShooterCharacter::TryReload);

		// 질주: 누른 순간 시작, 뗀 순간 끝 (점프와 같은 Started/Completed 패턴)
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AShooterCharacter::StartSprint);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopSprint);

		// 조준: 누르고 있는 동안만 (질주와 같은 패턴)
		EIC->BindAction(AimAction, ETriggerEvent::Started, this, &AShooterCharacter::StartAim);
		EIC->BindAction(AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopAim);
	}
}

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	// WASD → (X: 좌우, Y: 앞뒤)
	const FVector2D Axis = Value.Get<FVector2D>();

	if (Controller)
	{
		// 카메라가 보는 방향 기준으로 이동 (위아래 기울기는 무시하고 수평 회전만 사용)
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, Axis.Y);
		AddMovementInput(Right, Axis.X);
	}
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	// 마우스 이동량 → (X: 좌우, Y: 상하)
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AShooterCharacter::FireStarted()
{
	TryFire();
}

void AShooterCharacter::FireHeld()
{
	// 단발 무기는 누르고 있어도 추가 발사 없음 (다시 클릭해야 함)
	// 무기가 없으면 예전처럼 연사로 동작
	if (CurrentWeapon && !CurrentWeapon->IsAutomatic())
	{
		return;
	}
	TryFire();
}

void AShooterCharacter::TryFire()
{
	// 죽은 뒤에는 쏘지 않음
	if (IsDead())
	{
		return;
	}

	if (AbilitySystemComponent && FireAbility)
	{
		// 등록된 어빌리티 중 이 클래스를 찾아 실행 시도.
		// 쿨타임 중이거나 실행 조건이 안 맞으면 GAS가 알아서 거부한다 (반환값 false)
		const bool bFired = AbilitySystemComponent->TryActivateAbilityByClass(FireAbility);
		if (bFired)
		{
			LastFireTime = GetWorld()->GetTimeSeconds();
		}

		// 못 쐈는데 탄창이 비어 있으면 자동 재장전 (빈 총으로 클릭 = 재장전, 대부분의 슈터와 같은 동작)
		// 이미 재장전 중이면 GAS가 중복 실행을 거부하므로, 연사 버튼을 누르고 있어도 한 번만 시작된다
		if (!bFired && AbilitySystemComponent->GetNumericAttribute(UShooterAttributeSet::GetAmmoAttribute()) <= 0.f)
		{
			TryReload();
		}
	}
}

void AShooterCharacter::TryReload()
{
	if (AbilitySystemComponent && ReloadAbility && !IsDead())
	{
		// 탄창이 가득이거나 교체 중이면 GAS가 거부한다 (UShooterReloadAbility::CanActivateAbility, Blocked Tags)
		AbilitySystemComponent->TryActivateAbilityByClass(ReloadAbility);
	}
}

void AShooterCharacter::DebugDamage()
{
	if (!AbilitySystemComponent || !DebugDamageEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAS] DebugDamageEffect가 비어 있습니다. BP에서 GE_Damage를 지정하세요."));
		return;
	}

	// 1) Context: "누가, 무엇으로" 이 효과를 일으켰는지 담는 정보
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	// 2) Spec: GE 클래스(설계도) + 레벨 + Context를 묶은 "적용 준비가 끝난 실제 효과"
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DebugDamageEffect, 1.f, Context);

	// 3) 적용: 지금은 테스트라 자기 자신에게
	if (Spec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}
