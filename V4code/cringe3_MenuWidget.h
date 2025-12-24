#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Fonts/SlateFontInfo.h"

class ACringe3_MenuHUD;
class SEchoSettingsWidget;

FSlateFontInfo GetEchoFont(int32 Size);

class SMainMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainMenuWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<ACringe3_MenuHUD>, OwningHUD)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	void ProceedToMenu();
	void OnNewGameClicked();
	void OnQuitClicked();
	void OnSettingsClicked();
	void OnSettingsClosed();
	TSharedRef<SWidget> CreateMenuItem(const FString& ItemText, FSimpleDelegate OnClicked);

	TWeakObjectPtr<ACringe3_MenuHUD> OwningHUD;

	TSharedPtr<SWidget> PressAnyKeyContainer;
	TSharedPtr<SWidget> MainMenuContainer;
	TSharedPtr<class STextBlock> PressKeyText;

	TSharedPtr<SEchoSettingsWidget> SettingsWidget;

	bool bIsPressKeyScreen;
	float TimeAccumulator;
};

class SEchoMenuItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEchoMenuItem) : _ItemText(TEXT("Start")), _OnClicked() {}
		SLATE_ARGUMENT(FString, ItemText)
		SLATE_EVENT(FSimpleDelegate, OnClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	TSharedPtr<class STextBlock> TextBlock;
	TSharedPtr<class STextBlock> BarBlock;
	TSharedPtr<class SImage> GlowBackground; 

	FSimpleDelegate OnClickedDelegate;
	bool bIsHovered;
	float GlowAccumulator;
};