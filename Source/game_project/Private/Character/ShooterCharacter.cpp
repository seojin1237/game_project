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
	// 캡슐 중심 기준 앞 30, 오른쪽 25, 위 20 (대략 오른손 위치). 실제 조정은 BP 뷰포트에서
	WeaponAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponAttachPoint"));
	WeaponAttachPoint->SetupAttachment(RootComponent);
	WeaponAttachPoint->SetRelativeLocation(FVector(30.f, 25.f, 20.f));

	// ASC/AttributeSet 생성은 부모(AShooterCharacterBase) 생성자에서 이미 처리됨
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 기본 무기를 들고 시작
	if (DefaultWeaponClass)
	{
		EquipWeapon(DefaultWeaponClass);
	}
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
		AbilitySystemComponent->TryActivateAbilityByClass(FireAbility);
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
