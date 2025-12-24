#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "cringe3_topdownHUD.h"
#include "cringe3_topdownAIController.generated.h"

UENUM()
enum class EAICombatState : uint8
{
	Idle,       
	Chasing,    
	Attacking,  
	Retreating, 
	Stunned     
};

UCLASS()
class Acringe3_topdownAIController : public AAIController
{
	GENERATED_BODY()

public:
	Acringe3_topdownAIController();
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackRange = 180.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float TelegraphTime = 0.8f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float LoseInterestRadius = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "AI|Skills")
	float DashChance = 0.08f;

	UPROPERTY(EditAnywhere, Category = "AI|Skills")
	float RetreatChance = 0.4f;

private:
	FTimerHandle TelegraphTimerHandle;
	FTimerHandle ActionTimerHandle;
	FTimerHandle DecisionTimerHandle;

	EAICombatState CurrentAIState;
	ECombatDirection IntendedAttackDir;

	class Acringe3_topdownCharacter* ControlledChar;
	AActor* PlayerTarget;

	void UpdateStates(float DeltaSeconds);

	void StartAttackSequence();
	void ExecuteTelegraphedAttack();

	void DecideNextAction();
	void CheckForDash();
	void StartRetreat();

	void SetCombatFocus(bool bFocusOnPlayer);
};