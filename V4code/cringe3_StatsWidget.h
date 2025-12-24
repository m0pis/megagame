#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FEchoCharacterStats
{
	int32 STR;
	int32 DEX;
	int32 INT;
	int32 LUK;

	int32 AvailablePoints;
	int32 Level;

	float BaseHP = 100.0f;
	float BaseStamina = 100.0f;
	float BaseBarrier = 50.0f;

	FEchoCharacterStats()
		: STR(10), DEX(10), INT(10), LUK(10)
		, AvailablePoints(5), Level(1)
	{}
};

class SEchoStatsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEchoStatsWidget) : _OnClose() {}
		SLATE_EVENT(FSimpleDelegate, OnClose)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreateAttributeRow(const FString& Name, const FString& Code, int32* StatValuePtr, float GrowthFactor);
	TSharedRef<SWidget> CreateVitalRow(const FString& Name, const FString& Code, TSharedPtr<class STextBlock>& OutValueText);
	TSharedRef<SWidget> CreateInfoBlock(const FString& Title, const TArray<FString>& Lines);
	TSharedRef<SWidget> CreateActionCostRow(const FString& Action, const FString& Cost);

	void RecalculateStats();
	void OnIncreaseAttribute(int32* StatValuePtr);
	FReply OnBackClicked();
	FString GetCostText(int32 CurrentValue, float GrowthFactor) const;

	FSimpleDelegate OnCloseDelegate;
	FEchoCharacterStats Stats;

	TSharedPtr<class STextBlock> PointsText;

	TSharedPtr<class STextBlock> HPText;
	TSharedPtr<class STextBlock> StaminaText;
	TSharedPtr<class STextBlock> BarrierText;
	TSharedPtr<class STextBlock> ArmorText;
	TSharedPtr<class STextBlock> EvasionText;
	TSharedPtr<class STextBlock> DropRateText;

	TMap<int32*, TSharedPtr<class STextBlock>> StatValueWidgets;
	TMap<int32*, TSharedPtr<class STextBlock>> StatCostWidgets;
};