#include "SoulsCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ASoulsCharacter::ASoulsCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    //
    // настройка капсулы и движения
    //
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    //
    // SpringArm - просто замена упралением камеры, как в юнити делал, в анриле свое есть готовое
    //
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;

    //
    // сама камера
    //
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    bIsDashing = false;
    LockOnTarget = nullptr;
}

void ASoulsCharacter::BeginPlay()
{
    Super::BeginPlay();

    //
    // это маппинг, чтобы можно было запихать эдементы вввода, которые с помощью спец элементов создаются
    //
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ASoulsCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDashing)
    {
        DashTimer += DeltaTime;
        float Progress = DashTimer / CurrentDashDuration;

        float SpeedMultiplier = 1.0f;
        if (RollCurve)
        {
            SpeedMultiplier = RollCurve->GetFloatValue(Progress);
        }

        if (Progress >= 1.0f)
        {
            bIsDashing = false;
            GetCharacterMovement()->StopMovementImmediately();
        }
        else
        {
            FVector DashVelocity = DashDirection * CurrentDashSpeed * SpeedMultiplier;
            LaunchCharacter(DashVelocity, true, false);
        }
    }

    if (LockOnTarget)
    {
        float Distance = FVector::Dist(GetActorLocation(), LockOnTarget->GetActorLocation());
        if (Distance > LockOnRange)
        {
            ToggleLockOn();
            return;
        }

        FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LockOnTarget->GetActorLocation());
        FRotator CurrentRot = GetControlRotation();

        FRotator TargetRot = FMath::RInterpTo(CurrentRot, LookAtRot, DeltaTime, LockOnInterpSpeed);
        TargetRot.Pitch = FMath::Clamp(TargetRot.Pitch, -30.0f, 30.0f);

        if (Controller)
        {
            Controller->SetControlRotation(TargetRot);
        }

        if (!bIsDashing)
        {
            GetCharacterMovement()->bOrientRotationToMovement = false;
            FRotator ActorRot = GetActorRotation();
            FRotator TargetActorRot = FRotator(0, LookAtRot.Yaw, 0);
            SetActorRotation(FMath::RInterpTo(ActorRot, TargetActorRot, DeltaTime, RotationSpeed));
        }
    }
    else
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }
}

void ASoulsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoulsCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoulsCharacter::Look);
        EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ASoulsCharacter::PerformDash);
        EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &ASoulsCharacter::ToggleLockOn);
        EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ASoulsCharacter::ZoomCamera);
    }
}

void ASoulsCharacter::Move(const FInputActionValue& Value)
{
    if (bIsDashing) return;

    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ASoulsCharacter::Look(const FInputActionValue& Value)
{
    if (LockOnTarget) return;

    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ASoulsCharacter::PerformDash()
{
    if (bIsDashing || !GetCharacterMovement()->IsMovingOnGround()) return;

    FVector InputVector = GetLastMovementInputVector();

    if (InputVector.IsNearlyZero())
    {
        DashDirection = -GetActorForwardVector();
        CurrentDashDuration = BackstepDuration;
        CurrentDashSpeed = BackstepSpeed;
    }
    else
    {
        DashDirection = InputVector.GetSafeNormal();
        CurrentDashDuration = RollDuration;
        CurrentDashSpeed = RollSpeed;

        SetActorRotation(DashDirection.Rotation());
    }

    bIsDashing = true;
    DashTimer = 0.0f;
}

void ASoulsCharacter::ToggleLockOn()
{
    if (LockOnTarget)
    {
        LockOnTarget = nullptr;
    }
    else
    {
        FindClosestEnemy();
    }
}

void ASoulsCharacter::FindClosestEnemy()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    AActor* Closest = nullptr;
    float MinDist = LockOnRange;

    for (AActor* Actor : FoundActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < MinDist)
        {
            //
            // TODO: можно добавить проверку: виден ли враг (LineTrace)
            //
            MinDist = Dist;
            Closest = Actor;
        }
    }
    LockOnTarget = Closest;
}

void ASoulsCharacter::ZoomCamera(const FInputActionValue& Value)
{
    float ZoomValue = Value.Get<float>();

    if (ZoomValue != 0.0f)
    {
        float ZoomSpeed = 20.0f;

        float NewDistance = CameraBoom->TargetArmLength - (ZoomValue * ZoomSpeed);

        CameraBoom->TargetArmLength = FMath::Clamp(NewDistance, 150.0f, 800.0f);
    }
}