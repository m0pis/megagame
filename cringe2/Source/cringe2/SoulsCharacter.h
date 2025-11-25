#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SoulsCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCurveFloat;

UCLASS()
class CRINGE2_API ASoulsCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASoulsCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

    //
    // компоненты
    //
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    UCameraComponent* FollowCamera;

    //
    // настройки ввода (Enhanced Input)
    //
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* DashAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LockOnAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ZoomAction;

    //
    // параметры движения
    //
    UPROPERTY(EditAnywhere, Category = "Souls Movement")
    float RotationSpeed = 10.0f;

    //
    // Dash/Roll
    //
    UPROPERTY(EditAnywhere, Category = "Souls Dash")
    float RollSpeed = 1600.f;
    UPROPERTY(EditAnywhere, Category = "Souls Dash")
    float RollDuration = 0.4f;

    UPROPERTY(EditAnywhere, Category = "Souls Dash")
    float BackstepSpeed = 1400.f;
    UPROPERTY(EditAnywhere, Category = "Souls Dash")
    float BackstepDuration = 0.2f;

    //
    // кривая для управления скоростью и временем
    //
    UPROPERTY(EditAnywhere, Category = "Souls Dash")
    UCurveFloat* RollCurve;

    //
    // таргет лок
    //
    UPROPERTY(EditAnywhere, Category = "Souls LockOn")
    float LockOnRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Souls LockOn")
    float LockOnInterpSpeed = 10.0f;

private:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void PerformDash();
    void ToggleLockOn();
    void ZoomCamera(const FInputActionValue& Value);

    bool bIsDashing;
    float DashTimer;
    float CurrentDashDuration;
    float CurrentDashSpeed;
    FVector DashDirection;

    AActor* LockOnTarget;
    void FindClosestEnemy();
};