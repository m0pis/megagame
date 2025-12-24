#include "cringe3_topdownPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "cringe3_topdownCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"

Acringe3_topdownPlayerController::Acringe3_topdownPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	bIsAiming = false;
	SelectedDirection = ECombatDirection::Thrust;
}

void Acringe3_topdownPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
}

void Acringe3_topdownPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Acringe3_topdownPlayerController::OnMove);
		Input->BindAction(DashAction, ETriggerEvent::Started, this, &Acringe3_topdownPlayerController::OnDashTriggered);
		Input->BindAction(AttackExecuteAction, ETriggerEvent::Started, this, &Acringe3_topdownPlayerController::OnAttackExecute);
		Input->BindAction(AttackAimAction, ETriggerEvent::Started, this, &Acringe3_topdownPlayerController::OnAimStarted);
		Input->BindAction(AttackAimAction, ETriggerEvent::Triggered, this, &Acringe3_topdownPlayerController::OnAimTriggered);
		Input->BindAction(AttackAimAction, ETriggerEvent::Completed, this, &Acringe3_topdownPlayerController::OnAimCompleted);
		Input->BindAction(TargetLockAction, ETriggerEvent::Started, this, &Acringe3_topdownPlayerController::OnTargetLockTriggered);

		Input->BindAction(WalkAction, ETriggerEvent::Triggered, this, &Acringe3_topdownPlayerController::OnWalkStarted);
		Input->BindAction(WalkAction, ETriggerEvent::Completed, this, &Acringe3_topdownPlayerController::OnWalkStopped);

		Input->BindAction(BlockExecuteAction, ETriggerEvent::Triggered, this, &Acringe3_topdownPlayerController::OnBlockTriggered);
		Input->BindAction(BlockExecuteAction, ETriggerEvent::Completed, this, &Acringe3_topdownPlayerController::OnBlockCompleted);
		if (PauseAction)
		{
			Input->BindAction(PauseAction, ETriggerEvent::Started, this, &Acringe3_topdownPlayerController::OnPauseTriggered);
		}
	}
}

void Acringe3_topdownPlayerController::OnPauseTriggered()
{
	if (Acringe3_topdownHUD* HUD = Cast<Acringe3_topdownHUD>(GetHUD()))
	{
		HUD->TogglePauseMenu();
	}
}

void Acringe3_topdownPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(GetPawn());
	if (!MyChar) return;

	if (MyChar->IsTargetLocked())
	{
		return;
	}

	if (bIsAiming) return;

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, true, Hit))
	{
		FVector Start = MyChar->GetActorLocation();
		FVector Target = Hit.Location;
		Target.Z = Start.Z;

		FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(Start, Target);
		float Speed = (MyChar->GetCombatState() == ECombatState::Release) ? 5.0f : 25.0f;

		MyChar->SetActorRotation(FMath::RInterpTo(MyChar->GetActorRotation(), TargetRot, DeltaTime, Speed));
	}
}

void Acringe3_topdownPlayerController::OnTargetLockTriggered()
{
	Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(GetPawn());
	if (!MyChar) return;

	if (MyChar->IsTargetLocked())
	{
		MyChar->SetLockedTarget(nullptr);
	}
	else
	{
		AActor* BestTarget = FindBestTarget();
		if (BestTarget)
		{
			MyChar->SetLockedTarget(BestTarget);
		}
	}
}

AActor* Acringe3_topdownPlayerController::FindBestTarget()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return nullptr;

	FVector MyLoc = MyPawn->GetActorLocation();
	TArray<FHitResult> HitResults;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(1000.0f);
	GetWorld()->SweepMultiByChannel(HitResults, MyLoc, MyLoc, FQuat::Identity, ECC_Pawn, Sphere);

	AActor* ClosestActor = nullptr;
	float MinDistSq = FLT_MAX;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();

		if (!HitActor || HitActor == MyPawn) continue;

		Acringe3_topdownCharacter* TargetChar = Cast<Acringe3_topdownCharacter>(HitActor);
		if (!TargetChar) continue;

		if (TargetChar->GetCombatState() == ECombatState::Dead) continue;

		float DistSq = FVector::DistSquared(MyLoc, HitActor->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			ClosestActor = HitActor;
		}
	}
	return ClosestActor;
}

void Acringe3_topdownPlayerController::OnMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (APawn* ControlledPawn = GetPawn())
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void Acringe3_topdownPlayerController::OnAimStarted() { bIsAiming = true; UpdateAimDirection(); }
void Acringe3_topdownPlayerController::OnAimTriggered() { UpdateAimDirection(); }
void Acringe3_topdownPlayerController::OnAimCompleted() { bIsAiming = false; if (Acringe3_topdownHUD* HUD = Cast<Acringe3_topdownHUD>(GetHUD())) HUD->SetAimingMode(false, FVector2D::ZeroVector, FVector2D::ZeroVector); }
void Acringe3_topdownPlayerController::OnDashTriggered() { if (Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(GetPawn())) MyChar->StartDash(); }
void Acringe3_topdownPlayerController::OnAttackExecute() {
	if (Acringe3_topdownCharacter* C = Cast<Acringe3_topdownCharacter>(GetPawn())) {
		if (bIsAiming) C->ExecuteAttack(SelectedDirection);
		else C->ExecuteAttack(ECombatDirection::Thrust);
	}
}

void Acringe3_topdownPlayerController::OnBlockTriggered()
{
	Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(GetPawn());
	Acringe3_topdownHUD* HUD = Cast<Acringe3_topdownHUD>(GetHUD());

	if (!MyChar) return;

	if (bIsAiming)
	{
		UpdateAimDirection();
		MyChar->ExecuteBlock(SelectedDirection);
	}
	else
	{
		if (HUD)
		{
			HUD->SetAimingMode(false, FVector2D::ZeroVector, FVector2D::ZeroVector);
		}

		MyChar->ExecuteBlock(ECombatDirection::Thrust);
	}
}

void Acringe3_topdownPlayerController::OnBlockCompleted()
{
	Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(GetPawn());

	if (MyChar)
	{
		MyChar->EndBlock();
	}
}

void Acringe3_topdownPlayerController::UpdateAimDirection()
{
	Acringe3_topdownHUD* HUD = Cast<Acringe3_topdownHUD>(GetHUD());
	APawn* MyPawn = GetPawn();
	if (!HUD || !MyPawn) return;

	FVector2D CharScreenPos;
	ProjectWorldLocationToScreen(MyPawn->GetActorLocation(), CharScreenPos);
	FVector2D MousePos;
	GetMousePosition(MousePos.X, MousePos.Y);

	float Dist = FVector2D::Distance(CharScreenPos, MousePos);
	if (Dist > MaxAimMouseRadius) {
		FVector2D Dir = (MousePos - CharScreenPos).GetSafeNormal();
		FVector2D NewMousePos = CharScreenPos + (Dir * MaxAimMouseRadius);
		if (ULocalPlayer* LP = Cast<ULocalPlayer>(Player)) if (LP->ViewportClient) {
			LP->ViewportClient->Viewport->SetMouse(NewMousePos.X, NewMousePos.Y);
			MousePos = NewMousePos;
		}
	}

	FVector ForwardWorldPoint = MyPawn->GetActorLocation() + (MyPawn->GetActorForwardVector() * 200.0f);
	FVector2D ForwardScreenPos;
	ProjectWorldLocationToScreen(ForwardWorldPoint, ForwardScreenPos);
	FVector2D ScreenForwardDir = (ForwardScreenPos - CharScreenPos).GetSafeNormal();

	FVector2D MouseDir = (MousePos - CharScreenPos).GetSafeNormal();

	float ForwardAngleRad = FMath::Atan2(ScreenForwardDir.Y, ScreenForwardDir.X);
	float MouseAngleRad = FMath::Atan2(MouseDir.Y, MouseDir.X);

	float RelativeAngleRad = MouseAngleRad - ForwardAngleRad;
	RelativeAngleRad = FMath::Atan2(FMath::Sin(RelativeAngleRad), FMath::Cos(RelativeAngleRad));
	float RelativeAngleDeg = FMath::RadiansToDegrees(RelativeAngleRad);

	HUD->SetAimingMode(true, CharScreenPos, MousePos, ForwardAngleRad);

	int32 SectorIndex = FMath::RoundToInt(RelativeAngleDeg / 45.0f);

	if (SectorIndex < 0) SectorIndex += 8;
	SectorIndex = SectorIndex % 8;

	SelectedDirection = (ECombatDirection)SectorIndex;
	HUD->UpdateSelectedDirection(SelectedDirection);
}

void Acringe3_topdownPlayerController::OnWalkStarted()
{
	if (Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(GetPawn()))
	{
		MyChar->StartWalk();
	}
}

void Acringe3_topdownPlayerController::OnWalkStopped()
{
	if (Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(GetPawn()))
	{
		MyChar->StopWalk();
	}
}