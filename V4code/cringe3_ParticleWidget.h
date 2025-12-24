#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "UObject/GCObject.h"

struct FSimpleParticle
{
	FVector2D Position;
	FVector2D Velocity;
	float LifeTime;
	float MaxLifeTime;
	float Size;
	float Opacity;
};

class SParticleBackground : public SLeafWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SParticleBackground) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(1920, 1080); }

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("SParticleBackground"); }
	// ------------------------------------------

private:
	void SpawnParticle(bool bRandomStart = false);

	TArray<FSimpleParticle> Particles;
	double LastUpdateTime;

	FSlateBrush SmokeShaderBrush;

	TObjectPtr<class UMaterialInstanceDynamic> SmokeMID;

	float AccumulatedRealTime;

	TObjectPtr<class UMaterialInterface> SmokeMaterialObject;
};