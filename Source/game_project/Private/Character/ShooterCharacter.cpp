#include "Character/ShooterCharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/ShooterAttributeSet.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

AShooterCharacter::AShooterCharacter()
{
	// 아직 매 프레임 할 일이 없으므로 Tick 끔 (성능)
	PrimaryActorTick.bCanEverTick = false;

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

	// ===== GAS =====
	// 생성자에서 컴포넌트 생성 → 이 캐릭터의 기본 구성품이 된다
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AttributeSet도 ASC의 Owner(=이 캐릭터)의 서브오브젝트로 만들면
	// ASC가 초기화될 때 자동으로 찾아서 등록한다
	AttributeSet = CreateDefaultSubobject<UShooterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AShooterCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		// ASC에게 "누가 소유자(Owner)이고, 누가 실제로 움직이는 몸(Avatar)인지" 알려준다.
		// 이 캐릭터는 ASC를 직접 들고 있으므로 둘 다 this.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AShooterCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// 플레이어가 조종할 때만 (AI 컨트롤러면 Cast 실패 → 무시)
	// BP에서 IMC를 아직 지정 안 했으면 아무것도 안 함
	if (!DefaultMappingContext)
	{
		return;
	}

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
