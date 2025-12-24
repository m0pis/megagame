#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "cringe3_topdownHUD.h"
#include "cringe3_topdownPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class Acringe3_topdownPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	Acringe3_topdownPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackExecuteAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* TargetLockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* BlockExecuteAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* PauseAction;

	UPROPERTY(EditAnywhere, Category = "Combat Controls")
	float MaxAimMouseRadius = 150.0f;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	void OnMove(const FInputActionValue& Value);
	void OnDashTriggered();
	void OnAttackExecute();
	void OnBlockTriggered();
	void OnBlockCompleted();

	void OnAimStarted();
	void OnAimTriggered();
	void OnAimCompleted();

	void OnTargetLockTriggered();

	void OnWalkStarted();
	void OnWalkStopped();

	void OnPauseTriggered();

private:
	bool bIsAiming;
	ECombatDirection SelectedDirection;

	void UpdateAimDirection();
	AActor* FindBestTarget();
};