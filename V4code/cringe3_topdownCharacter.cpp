#include "cringe3_topdownCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

Acringe3_topdownCharacter::Acringe3_topdownCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 950.f;
	CameraBoom->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	DefaultRunSpeed = GetCharacterMovement()->MaxWalkSpeed;
	DefaultCameraLagSpeed = CameraBoom->CameraLagSpeed;
	CombatState = ECombatState::Idle;
	LockedTarget = nullptr;
	bIsWalking = false;
	bIsBlocking = false;

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	BlockStartTime = 0.0f;
	LastDashTime = -10.0f;
}

void Acringe3_topdownCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CombatState == ECombatState::Dead) return;

	if (CombatState != ECombatState::Idle && CombatState != ECombatState::Recovery && CombatState != ECombatState::Stunned)
	{
		CombatStateTimer += DeltaSeconds;
		if (CombatState == ECombatState::Release)
		{
			ProcessWeaponCollision(DeltaSeconds);
		}
	}

	UpdateTargetLock(DeltaSeconds);
	RegenerateStamina(DeltaSeconds);
}

void Acringe3_topdownCharacter::RegenerateStamina(float DeltaTime)
{
	if (bIsBlocking || bIsDashing || CombatState == ECombatState::Windup || CombatState == ECombatState::Release)
	{
		return;
	}

	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina += StaminaRegenRate * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}
}

float Acringe3_topdownCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CombatState == ECombatState::Dead) return 0.0f;
	CurrentHealth -= DamageAmount;
	if (CurrentHealth <= 0.0f) Die();
	return DamageAmount;
}

void Acringe3_topdownCharacter::ReceiveDirectionalDamage(float Damage, ECombatDirection AttackDir, AActor* DamageCauser)
{
	if (CombatState == ECombatState::Dead) return;

	bool bDidBlock = false;
	bool bDidParry = false;

	if (bIsBlocking && CombatState == ECombatState::Idle)
	{
		ECombatDirection RequiredBlockDir = ECombatDirection::Thrust;

		switch (AttackDir)
		{
		case ECombatDirection::Top:         RequiredBlockDir = ECombatDirection::Top; break;
		case ECombatDirection::Bottom:      RequiredBlockDir = ECombatDirection::Bottom; break;
		case ECombatDirection::Left:        RequiredBlockDir = ECombatDirection::Right; break;
		case ECombatDirection::Right:       RequiredBlockDir = ECombatDirection::Left; break;
		case ECombatDirection::TopLeft:     RequiredBlockDir = ECombatDirection::TopRight; break;
		case ECombatDirection::TopRight:    RequiredBlockDir = ECombatDirection::TopLeft; break;
		case ECombatDirection::BottomLeft:  RequiredBlockDir = ECombatDirection::BottomRight; break;
		case ECombatDirection::BottomRight: RequiredBlockDir = ECombatDirection::BottomLeft; break;
		case ECombatDirection::Thrust:      RequiredBlockDir = ECombatDirection::Thrust; break;
		}

		if ((AttackDir == ECombatDirection::Thrust && CurrentBlockDir == ECombatDirection::Thrust) ||
			(AttackDir != ECombatDirection::Thrust && CurrentBlockDir == RequiredBlockDir))
		{
			FVector ToAttacker = DamageCauser->GetActorLocation() - GetActorLocation();
			ToAttacker.Normalize();
			float Dot = FVector::DotProduct(GetActorForwardVector(), ToAttacker);

			if (Dot > 0.2f)
			{
				bDidBlock = true;
			}
		}
	}

	if (bDidBlock && DamageCauser)
	{
		double TimeSinceBlockStart = GetWorld()->GetTimeSeconds() - BlockStartTime;

		if (TimeSinceBlockStart <= ParryWindowTime)
		{
			bDidParry = true;

			CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaCostParry);

			FString Msg = TEXT("PARRY!");
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, Msg);

			if (Acringe3_topdownCharacter* Attacker = Cast<Acringe3_topdownCharacter>(DamageCauser))
			{
				Attacker->OnWasParried();
			}
		}
		else
		{
			if (CurrentStamina >= StaminaCostBlockHit)
			{
				CurrentStamina -= StaminaCostBlockHit;

				FString Msg = TEXT("BLOCKED");
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Msg);
			}
			else
			{
				bDidBlock = false;
				CurrentStamina = 0.0f;

				FString Msg = TEXT("GUARD BROKEN!");
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, Msg);

				ResetCombat();
				CombatState = ECombatState::Recovery;
				GetWorldTimerManager().SetTimer(StateTimerHandle, this, &Acringe3_topdownCharacter::ResetCombat, 1.0f, false);
			}
		}
	}

	if (!bDidBlock)
	{
		CurrentHealth -= Damage;

		FString Msg = FString::Printf(TEXT("HIT | Damage: %.1f | HP: %.1f"), Damage, CurrentHealth);
		UE_LOG(LogTemp, Error, TEXT("%s"), *Msg);

		if (CurrentHealth <= 0.0f)
		{
			Die();
		}
		else
		{
			if (CombatState != ECombatState::Idle && CombatState != ECombatState::Recovery && CombatState != ECombatState::Stunned)
			{
				ResetCombat();
				CombatState = ECombatState::Recovery;
				GetWorldTimerManager().SetTimer(StateTimerHandle, this, &Acringe3_topdownCharacter::ResetCombat, 0.4f, false);
			}
		}
	}
}

void Acringe3_topdownCharacter::OnWasParried()
{
	ResetCombat();
	CombatState = ECombatState::Stunned;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, TEXT("YOU WERE PARRIED!"));

	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &Acringe3_topdownCharacter::ResetCombat, 2.0f, false);
}

void Acringe3_topdownCharacter::Die()
{
	if (CombatState == ECombatState::Dead) return;
	CombatState = ECombatState::Dead;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	if (Controller) Controller->UnPossess();
}

void Acringe3_topdownCharacter::StartWalk()
{
	bIsWalking = true;
	if (!bIsDashing && CombatState == ECombatState::Idle) GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void Acringe3_topdownCharacter::StopWalk()
{
	bIsWalking = false;
	if (!bIsDashing && CombatState == ECombatState::Idle) GetCharacterMovement()->MaxWalkSpeed = DefaultRunSpeed;
}

void Acringe3_topdownCharacter::UpdateTargetLock(float DeltaTime)
{
	if (!LockedTarget)
	{
		CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, FVector::ZeroVector, DeltaTime, 3.0f);
		return;
	}

	Acringe3_topdownCharacter* TargetChar = Cast<Acringe3_topdownCharacter>(LockedTarget);
	if (TargetChar && TargetChar->GetCombatState() == ECombatState::Dead)
	{
		SetLockedTarget(nullptr);
		return;
	}

	float Dist = GetDistanceTo(LockedTarget);
	if (Dist > MaxLockDistance || LockedTarget->IsHidden())
	{
		SetLockedTarget(nullptr);
		return;
	}

	if (!bIsDashing)
	{
		FVector Start = GetActorLocation();
		FVector Target = LockedTarget->GetActorLocation();
		Target.Z = Start.Z;
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(Start, Target);
		FRotator NewRot = FMath::RInterpTo(GetActorRotation(), LookAtRot, DeltaTime, 15.0f);
		SetActorRotation(NewRot);
	}

	FVector DirToTarget = (LockedTarget->GetActorLocation() - GetActorLocation());
	FRotator BoomRot = CameraBoom->GetComponentRotation();
	FVector RotatedOffset = BoomRot.UnrotateVector(DirToTarget * 0.3f);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, RotatedOffset, DeltaTime, 2.0f);
}

void Acringe3_topdownCharacter::SetLockedTarget(AActor* NewTarget)
{
	LockedTarget = NewTarget;
}

void Acringe3_topdownCharacter::ExecuteAttack(ECombatDirection Direction)
{
	if (CombatState != ECombatState::Idle && CombatState != ECombatState::Recovery) return;
	if (bIsBlocking) return;

	if (CurrentStamina < StaminaCostAttack)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Low Stamina!"));
		return;
	}

	CurrentStamina -= StaminaCostAttack;
	CurrentAttackDir = Direction;
	EnterWindup();
}

void Acringe3_topdownCharacter::ExecuteBlock(ECombatDirection Direction)
{
	bool bCanBlock = (CombatState == ECombatState::Idle || CombatState == ECombatState::Recovery || bIsBlocking);

	if (!bCanBlock || CombatState == ECombatState::Windup || CombatState == ECombatState::Release || CombatState == ECombatState::Dead || CombatState == ECombatState::Stunned) return;

	if (!bIsBlocking)
	{
		BlockStartTime = GetWorld()->GetTimeSeconds();
	}

	CurrentBlockDir = Direction;
	bIsBlocking = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

bool Acringe3_topdownCharacter::IsBlocking() const
{
	return bIsBlocking;
}

void Acringe3_topdownCharacter::EndBlock()
{
	bIsBlocking = false;
	if (!bIsWalking && CombatState == ECombatState::Idle)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultRunSpeed;
	}
}

void Acringe3_topdownCharacter::EnterWindup()
{
	CombatState = ECombatState::Windup;
	CombatStateTimer = 0.0f;
	HitActorsCurrentSwing.Empty();
	PreviousFrameBladePoints.Empty();
	for (int i = 0; i < 5; i++) PreviousFrameBladePoints.Add(FVector::ZeroVector);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	CalculateTrajectoryParams();
	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &Acringe3_topdownCharacter::EnterRelease, WindupTime, false);
}

void Acringe3_topdownCharacter::EnterRelease()
{
	CombatState = ECombatState::Release;
	CombatStateTimer = 0.0f;
	for (int32 i = 0; i < 5; i++)
	{
		float Pct = (float)i / 4.0f;
		PreviousFrameBladePoints[i] = GetBladePointLocation(Pct, 0.0f);
	}
	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &Acringe3_topdownCharacter::EnterRecovery, ReleaseTime, false);
}

void Acringe3_topdownCharacter::EnterRecovery()
{
	CombatState = ECombatState::Recovery;
	CombatStateTimer = 0.0f;
	GetWorldTimerManager().SetTimer(StateTimerHandle, this, &Acringe3_topdownCharacter::ResetCombat, RecoveryTime, false);
}

void Acringe3_topdownCharacter::ResetCombat()
{
	CombatState = ECombatState::Idle;
	HitActorsCurrentSwing.Empty();
	if (!bIsDashing && !bIsBlocking)
	{
		GetCharacterMovement()->MaxWalkSpeed = bIsWalking ? WalkSpeed : DefaultRunSpeed;
	}
}

void Acringe3_topdownCharacter::CalculateTrajectoryParams()
{
	float SwingArc = 120.0f;
	float VerticalArc = 70.0f;
	switch (CurrentAttackDir)
	{
	case ECombatDirection::Left: StartYaw = -SwingArc / 2; EndYaw = SwingArc / 2; StartPitch = 0.f; EndPitch = 0.f; break;
	case ECombatDirection::Right: StartYaw = SwingArc / 2; EndYaw = -SwingArc / 2; StartPitch = 0.f; EndPitch = 0.f; break;
	case ECombatDirection::Top: StartYaw = 0.f; EndYaw = 0.f; StartPitch = VerticalArc; EndPitch = -VerticalArc; break;
	case ECombatDirection::Bottom: StartYaw = 0.f; EndYaw = 0.f; StartPitch = -VerticalArc; EndPitch = VerticalArc; break;
	case ECombatDirection::TopRight: StartYaw = SwingArc / 2; EndYaw = -SwingArc / 2; StartPitch = VerticalArc; EndPitch = -VerticalArc; break;
	case ECombatDirection::TopLeft: StartYaw = -SwingArc / 2; EndYaw = SwingArc / 2; StartPitch = VerticalArc; EndPitch = -VerticalArc; break;
	case ECombatDirection::BottomRight: StartYaw = SwingArc / 2; EndYaw = -SwingArc / 2; StartPitch = -VerticalArc; EndPitch = VerticalArc; break;
	case ECombatDirection::BottomLeft: StartYaw = -SwingArc / 2; EndYaw = SwingArc / 2; StartPitch = -VerticalArc; EndPitch = VerticalArc; break;
	case ECombatDirection::Thrust: StartYaw = 0; EndYaw = 0; StartPitch = 0; EndPitch = 0; break;
	}
}

FVector Acringe3_topdownCharacter::GetBladePointLocation(float PercentageAlongBlade, float SwingProgress)
{
	if (CurrentAttackDir == ECombatDirection::Thrust)
	{
		float StabReach = FMath::Sin(SwingProgress * PI);
		FVector StartPos = GetActorLocation() + FVector(0, 0, 50) + (GetActorRightVector() * 20.0f);
		FVector AimDir = GetActorForwardVector();
		return FMath::Lerp(StartPos + (AimDir * (StabReach * 50.0f)), StartPos + (AimDir * (WeaponLength + (StabReach * 100.0f))), PercentageAlongBlade);
	}
	float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, SwingProgress, 2.0f);
	float CurrentYaw = FMath::Lerp(StartYaw, EndYaw, EasedAlpha);
	float CurrentPitch = FMath::Lerp(StartPitch, EndPitch, EasedAlpha);
	FRotator ActorRot = GetActorRotation();
	FRotator SwingRot = FRotator(CurrentPitch, ActorRot.Yaw + CurrentYaw, 0.0f);
	FVector SwingDir = SwingRot.Vector();
	FVector Pivot = GetActorLocation() + FVector(0, 0, 50.0f);
	return Pivot + (SwingDir * 20.0f) + (SwingDir * (WeaponLength * PercentageAlongBlade));
}

void Acringe3_topdownCharacter::ProcessWeaponCollision(float DeltaTime)
{
	float Progress = FMath::Clamp(CombatStateTimer / ReleaseTime, 0.0f, 1.0f);
	const int32 NumTracePoints = 5;
	for (int32 i = 0; i < NumTracePoints; i++)
	{
		float Percent = (float)i / (float)(NumTracePoints - 1);
		FVector CurrentLoc = GetBladePointLocation(Percent, Progress);
		FVector PreviousLoc = PreviousFrameBladePoints[i];
		if (i == NumTracePoints - 1) DrawSwingDebug(PreviousLoc, CurrentLoc);

		TArray<FHitResult> HitResults;
		FCollisionQueryParams Params; Params.AddIgnoredActor(this);

		if (GetWorld()->SweepMultiByChannel(HitResults, PreviousLoc, CurrentLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(15.0f), Params))
		{
			for (const FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				if (HitActor && !HitActorsCurrentSwing.Contains(HitActor))
				{
					HitActorsCurrentSwing.Add(HitActor);
					float Velocity = FVector::Dist(PreviousLoc, CurrentLoc) / (DeltaTime > 0 ? DeltaTime : 0.01f);
					float DamageMult = (Velocity > 800.f) ? 1.2f : 0.8f;
					if (CurrentAttackDir == ECombatDirection::Thrust) DamageMult = 1.5f;

					Acringe3_topdownCharacter* Victim = Cast<Acringe3_topdownCharacter>(HitActor);
					if (Victim)
					{
						Victim->ReceiveDirectionalDamage(BaseDamage * DamageMult, CurrentAttackDir, this);
					}
					else
					{
						UGameplayStatics::ApplyDamage(HitActor, BaseDamage * DamageMult, GetController(), this, UDamageType::StaticClass());
					}
					DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 15.0f, FColor::Red, false, 2.0f);
				}
			}
		}
		PreviousFrameBladePoints[i] = CurrentLoc;
	}
}

void Acringe3_topdownCharacter::DrawSwingDebug(const FVector& Start, const FVector& End)
{
	DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 0.5f, 0, 2.0f);
}

void Acringe3_topdownCharacter::StartDash()
{
	if (bIsDashing) return;
	if (CombatState == ECombatState::Dead) return; 
	if (CombatState == ECombatState::Stunned) return; 

	double CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastDashTime < DashCooldown)
	{
		if (IsPlayerControlled() && GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, TEXT("Dash Cooldown"));
		return;
	}

	if (CurrentStamina < StaminaCostDash)
	{
		if (IsPlayerControlled() && GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, TEXT("Low Stamina"));
		return;
	}


	LastDashTime = CurrentTime;
	CurrentStamina -= StaminaCostDash;
	bIsDashing = true;

	FVector DashDir = GetVelocity().GetSafeNormal();

	if (DashDir.IsNearlyZero())
	{
		DashDir = -GetActorForwardVector();
	}

	GetCharacterMovement()->Velocity = FVector::ZeroVector;

	GetCharacterMovement()->Velocity = DashDir * DashVelocity;

	GetCharacterMovement()->MaxWalkSpeed = DashVelocity;
	GetCharacterMovement()->MaxAcceleration = 60000.f;

	GetWorldTimerManager().SetTimer(DashTimerHandle, this, &Acringe3_topdownCharacter::StopDash, DashDistanceTime, false);
}

void Acringe3_topdownCharacter::StopDash()
{
	bIsDashing = false;
	if (CombatState == ECombatState::Idle) GetCharacterMovement()->MaxWalkSpeed = bIsWalking ? WalkSpeed : DefaultRunSpeed;
	else GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxAcceleration = 2048.f;
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
}