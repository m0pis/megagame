#include "cringe3_topdownAIController.h"
#include "cringe3_topdownCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"

Acringe3_topdownAIController::Acringe3_topdownAIController()
{
	CurrentAIState = EAICombatState::Idle;
}

void Acringe3_topdownAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledChar = Cast<Acringe3_topdownCharacter>(InPawn);

	GetWorldTimerManager().SetTimer(DecisionTimerHandle, this, &Acringe3_topdownAIController::CheckForDash, 0.5f, true);
}

void Acringe3_topdownAIController::OnUnPossess()
{
	Super::OnUnPossess();
	ControlledChar = nullptr;
	PlayerTarget = nullptr;
	GetWorldTimerManager().ClearTimer(DecisionTimerHandle);
	GetWorldTimerManager().ClearTimer(TelegraphTimerHandle);
	GetWorldTimerManager().ClearTimer(ActionTimerHandle);
}

void Acringe3_topdownAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ControlledChar || ControlledChar->GetCombatState() == ECombatState::Dead)
	{
		StopMovement();
		SetCombatFocus(false);
		return;
	}

	if (ControlledChar->GetCombatState() == ECombatState::Stunned)
	{
		CurrentAIState = EAICombatState::Stunned;
		StopMovement();
		return;
	}
	else if (CurrentAIState == EAICombatState::Stunned && ControlledChar->GetCombatState() == ECombatState::Idle)
	{
		CurrentAIState = EAICombatState::Chasing;
	}

	if (!PlayerTarget)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn && FVector::DistSquared(ControlledChar->GetActorLocation(), PlayerPawn->GetActorLocation()) < DetectionRadius * DetectionRadius)
		{
			PlayerTarget = PlayerPawn;
			CurrentAIState = EAICombatState::Chasing;
		}
	}
	else
	{
		float DistSq = FVector::DistSquared(ControlledChar->GetActorLocation(), PlayerTarget->GetActorLocation());
		if (DistSq > LoseInterestRadius * LoseInterestRadius)
		{
			PlayerTarget = nullptr;
			CurrentAIState = EAICombatState::Idle;
			StopMovement();
			SetCombatFocus(false);
		}
	}

	UpdateStates(DeltaSeconds);
}

void Acringe3_topdownAIController::SetCombatFocus(bool bFocusOnPlayer)
{
	if (!ControlledChar) return;

	if (bFocusOnPlayer && PlayerTarget)
	{
		SetFocus(PlayerTarget);

		ControlledChar->bUseControllerRotationYaw = true;

		ControlledChar->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);

		ControlledChar->bUseControllerRotationYaw = false;
		ControlledChar->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void Acringe3_topdownAIController::UpdateStates(float DeltaSeconds)
{
	if (!PlayerTarget) return;

	float DistToPlayer = FVector::Dist(ControlledChar->GetActorLocation(), PlayerTarget->GetActorLocation());

	switch (CurrentAIState)
	{
	case EAICombatState::Idle:
		SetCombatFocus(false);
		break;

	case EAICombatState::Chasing:
	{
		bool bCombatMode = (DistToPlayer < 600.0f);
		SetCombatFocus(bCombatMode);

		if (DistToPlayer <= AttackRange)
		{
			StopMovement();
			CurrentAIState = EAICombatState::Attacking;
			StartAttackSequence();
		}
		else
		{
			MoveToActor(PlayerTarget, AttackRange * 0.8f);
		}
	}
	break;

	case EAICombatState::Attacking:
	{
		SetCombatFocus(true);

		if (GetWorldTimerManager().IsTimerActive(TelegraphTimerHandle))
		{
			FVector Start = ControlledChar->GetActorLocation() + FVector(0, 0, 50);
			FVector Forward = ControlledChar->GetActorForwardVector();
			FVector Right = ControlledChar->GetActorRightVector();
			FVector DirVec = Forward;

			switch (IntendedAttackDir)
			{
			case ECombatDirection::Top:         DirVec = Forward; break;
			case ECombatDirection::Bottom:      DirVec = -Forward; break;
			case ECombatDirection::Left:        DirVec = -Right; break;
			case ECombatDirection::Right:       DirVec = Right; break;
			case ECombatDirection::TopLeft:     DirVec = (Forward - Right).GetSafeNormal(); break;
			case ECombatDirection::TopRight:    DirVec = (Forward + Right).GetSafeNormal(); break;
			case ECombatDirection::BottomLeft:  DirVec = (-Forward - Right).GetSafeNormal(); break;
			case ECombatDirection::BottomRight: DirVec = (-Forward + Right).GetSafeNormal(); break;
			case ECombatDirection::Thrust:      DirVec = Forward; break;
			}

			FVector End = Start + (DirVec * 200.0f);
			DrawDebugDirectionalArrow(GetWorld(), Start, End, 40.0f, FColor::Red, false, -1.0f, 0, 5.0f);
		}
	}
	break;

	case EAICombatState::Retreating:
	{
		SetCombatFocus(true);

		if (DistToPlayer > 700.0f)
		{
			CurrentAIState = EAICombatState::Chasing;
		}
	}
	break;

	case EAICombatState::Stunned:
		SetCombatFocus(true);
		break;
	}
}

void Acringe3_topdownAIController::CheckForDash()
{
	if (!ControlledChar || !PlayerTarget) return;
	if (CurrentAIState == EAICombatState::Idle || CurrentAIState == EAICombatState::Stunned) return;

	if (FMath::FRand() < DashChance)
	{
		ControlledChar->StartDash();
	}
}

void Acringe3_topdownAIController::StartAttackSequence()
{
	if (!ControlledChar || CurrentAIState != EAICombatState::Attacking) return;

	int32 RandomDir = FMath::RandRange(0, 8);
	IntendedAttackDir = (ECombatDirection)RandomDir;

	GetWorldTimerManager().SetTimer(TelegraphTimerHandle, this, &Acringe3_topdownAIController::ExecuteTelegraphedAttack, TelegraphTime, false);
}

void Acringe3_topdownAIController::ExecuteTelegraphedAttack()
{
	if (CurrentAIState == EAICombatState::Attacking && ControlledChar && ControlledChar->GetCombatState() != ECombatState::Stunned)
	{
		ControlledChar->ExecuteAttack(IntendedAttackDir);

		GetWorldTimerManager().SetTimer(ActionTimerHandle, this, &Acringe3_topdownAIController::DecideNextAction, 1.5f, false);
	}
}

void Acringe3_topdownAIController::DecideNextAction()
{
	if (!ControlledChar || !PlayerTarget) return;
	if (ControlledChar->GetCombatState() == ECombatState::Stunned) return;

	if (FMath::FRand() < RetreatChance)
	{
		StartRetreat();
	}
	else
	{
		CurrentAIState = EAICombatState::Chasing;
	}
}

void Acringe3_topdownAIController::StartRetreat()
{
	CurrentAIState = EAICombatState::Retreating;

	FVector DirFromPlayer = (ControlledChar->GetActorLocation() - PlayerTarget->GetActorLocation()).GetSafeNormal();

	FVector RetreatPos = ControlledChar->GetActorLocation() + (DirFromPlayer * 600.0f);

	MoveToLocation(RetreatPos, 50.0f);

	GetWorldTimerManager().SetTimer(ActionTimerHandle, [this]()
		{
			if (CurrentAIState == EAICombatState::Retreating)
			{
				CurrentAIState = EAICombatState::Chasing;
			}
		}, 2.5f, false);
}