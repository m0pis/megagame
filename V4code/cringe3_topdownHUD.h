#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Widgets/SCompoundWidget.h"
#include "cringe3_topdownHUD.generated.h"

UENUM(BlueprintType)
enum class ECombatDirection : uint8
{
	Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, TopLeft, Thrust
};

class Acringe3_topdownHUD;
class SEchoPauseWidget;

class SEchoInGameHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEchoInGameHUD) {}
		SLATE_ARGUMENT(TWeakObjectPtr<Acringe3_topdownHUD>, OwningHUD)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TWeakObjectPtr<Acringe3_topdownHUD> OwnerHUD;
	void DrawArc(FSlateWindowElementList& OutDrawElements, int32 LayerId, const FGeometry& AllottedGeometry, FVector2D Center, float InnerRadius, float OuterRadius, float StartAngleDeg, float EndAngleDeg, FLinearColor Color, bool bIsOutline = false) const;
};

UCLASS()
class Acringe3_topdownHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void DrawHUD() override {}

	void SetAimingMode(bool bEnable, FVector2D Center, FVector2D MousePos, float AimAngleOffset = 0.0f);
	void UpdateSelectedDirection(ECombatDirection NewDir);

	void TogglePauseMenu();

	bool IsAiming() const { return bIsAiming; }
	FVector2D GetScreenCenter() const { return ScreenCenter; }
	FVector2D GetMousePos() const { return CurrentMousePos; }
	float GetRotationOffset() const { return CurrentRotationOffset; }
	ECombatDirection GetSelectedDir() const { return SelectedDirection; }

protected:
	bool bIsAiming;
	FVector2D ScreenCenter;
	FVector2D CurrentMousePos;
	float CurrentRotationOffset;
	ECombatDirection SelectedDirection;

	TSharedPtr<SEchoInGameHUD> HUDWidget;

	TSharedPtr<SEchoPauseWidget> PauseWidget;
	bool bIsPauseMenuVisible;
};