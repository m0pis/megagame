#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "GameFramework/GameUserSettings.h"

enum class ESettingType : uint8
{
	List,
	Slider
};

enum class ESettingsTab : uint8
{
	Video,
	AdvancedGraphics,
	Audio,   
	Gameplay 
};

DECLARE_DELEGATE_OneParam(FOnIntSettingChanged, int32);
DECLARE_DELEGATE_OneParam(FOnFloatSettingChanged, float);

struct FEchoSettingItemData
{
	FString Name;
	FString Description;
	ESettingType Type;

	TArray<FString> Options;
	int32 CurrentOptionIndex;
	FOnIntSettingChanged OnOptionChanged;

	float MinValue;
	float MaxValue;
	float CurrentValue;
	float Step;
	FOnFloatSettingChanged OnSliderChanged;

	FEchoSettingItemData(FString InName, FString InDesc, TArray<FString> InOptions, int32 DefaultIdx, FOnIntSettingChanged InFunc)
		: Name(InName), Description(InDesc), Type(ESettingType::List), Options(InOptions), CurrentOptionIndex(DefaultIdx), OnOptionChanged(InFunc), MinValue(0), MaxValue(0), CurrentValue(0), Step(0) {}

	FEchoSettingItemData(FString InName, FString InDesc, float Min, float Max, float DefaultVal, float InStep, FOnFloatSettingChanged InFunc)
		: Name(InName), Description(InDesc), Type(ESettingType::Slider), MinValue(Min), MaxValue(Max), CurrentValue(DefaultVal), Step(InStep), OnSliderChanged(InFunc) {}
};

class SEchoSettingsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEchoSettingsWidget) : _OnClose() {}
		SLATE_EVENT(FSimpleDelegate, OnClose)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void OnSettingHovered(TSharedPtr<FEchoSettingItemData> ItemData);
	void OnSettingUnhovered();

private:
	void InitializeSettings();
	void InitVideoSettings();
	void InitGraphicsSettings();
	void InitAudioSettings();    
	void InitGameplaySettings(); 

	TSharedRef<SWidget> BuildTabs();
	TSharedRef<SWidget> BuildTabButton(ESettingsTab TabID, const FString& Text);
	void RebuildSettingsList();
	TSharedRef<SWidget> CreateSettingRow(const TSharedPtr<FEchoSettingItemData>& ItemData);

	void OnTabClicked(ESettingsTab NewTab);
	FReply OnBackClicked();
	FReply OnApplyClicked();

	UGameUserSettings* GetGUS() const;

	FSimpleDelegate OnCloseDelegate;
	ESettingsTab CurrentTab;
	TMap<ESettingsTab, TArray<TSharedPtr<FEchoSettingItemData>>> SettingsMap;

	TSharedPtr<class SVerticalBox> SettingsListContainer;
	TSharedPtr<class STextBlock> DescriptionTitle;
	TSharedPtr<class STextBlock> DescriptionBody;

	TMap<ESettingsTab, TSharedPtr<class STextBlock>> TabTextWidgets;
};