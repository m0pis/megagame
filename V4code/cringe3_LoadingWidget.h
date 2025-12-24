#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SLoadingWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	float RotationAccumulator;
	float PulseAccumulator;

	TSharedPtr<class SImage> OuterRing;
	TSharedPtr<class SImage> InnerRing;
};