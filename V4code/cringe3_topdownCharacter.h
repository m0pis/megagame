#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "cringe3_topdownHUD.h"
#include "cringe3_topdownCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle,
	Windup,
	Release,
	Recovery,
	Dead,
	Stunned
};

UCLASS(Blueprintable)
class Acringe3_topdownCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	Acringe3_topdownCharacter();
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void ReceiveDirectionalDamage(float Damage, ECombatDirection AttackDir, AActor* DamageCauser);

	void OnWasParried();

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	void ExecuteAttack(ECombatDirection Direction);
	void ExecuteBlock(ECombatDirection Direction);
	bool IsBlocking() const;
	void EndBlock();

	void StartDash();
	void StopDash();

	void StartWalk();
	void StopWalk();

	void SetLockedTarget(AActor* NewTarget);
	AActor* GetLockedTarget() const { return LockedTarget; }
	bool IsTargetLocked() const { return LockedTarget != nullptr; }

	ECombatDirection GetCurrentBlockMode() const { return CurrentBlockDir; }
	ECombatState GetCombatState() const { return CombatState; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercent() const { return (MaxHealth > 0.f) ? CurrentHealth / MaxHealth : 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStaminaPercent() const { return (MaxStamina > 0.f) ? CurrentStamina / MaxStamina : 0.0f; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, Category = "Stats|Stamina")
	float StaminaRegenRate = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Stats|Stamina")
	float StaminaCostAttack = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Stats|Stamina")
	float StaminaCostBlockHit = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Stats|Stamina")
	float StaminaCostParry = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Stats|Stamina")
	float StaminaCostDash = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Combat Settings")
	float BaseDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Combat Settings")
	float WeaponLength = 160.0f;

	UPROPERTY(EditAnywhere, Category = "Combat Settings")
	float ParryWindowTime = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float WalkSpeed = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Target Lock")
	float MaxLockDistance = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Abilities|Dash")
	float DashDistanceTime = 0.12f;
	UPROPERTY(EditAnywhere, Category = "Abilities|Dash")
	float DashVelocity = 3000.f;
	UPROPERTY(EditAnywhere, Category = "Abilities|Dash")
	float DashCooldown = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat Settings")
	float WindupTime = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Combat Settings")
	float ReleaseTime = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Combat Settings")
	float RecoveryTime = 0.3f;

private:
	ECombatState CombatState;
	ECombatDirection CurrentAttackDir;
	ECombatDirection CurrentBlockDir;
	float CombatStateTimer;

	UPROPERTY()
	AActor* LockedTarget;

	bool bIsWalking;
	bool bIsBlocking;
	double BlockStartTime; 
	double LastDashTime;   

	float DefaultCameraLagSpeed;
	float StartPitch, EndPitch;
	float StartYaw, EndYaw;

	TArray<FVector> PreviousFrameBladePoints;
	TArray<AActor*> HitActorsCurrentSwing;

	FTimerHandle StateTimerHandle;
	FTimerHandle DashTimerHandle;
	float DefaultRunSpeed;
	bool bIsDashing;

	void EnterWindup();
	void EnterRelease();
	void EnterRecovery();
	void ResetCombat();
	void Die();

	void UpdateTargetLock(float DeltaTime);
	void RegenerateStamina(float DeltaTime);

	void CalculateTrajectoryParams();
	FVector GetBladePointLocation(float PercentageAlongBlade, float SwingProgress);
	void ProcessWeaponCollision(float DeltaTime);

	void DrawSwingDebug(const FVector& Start, const FVector& End);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};