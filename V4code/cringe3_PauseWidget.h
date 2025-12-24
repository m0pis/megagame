#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class Acringe3_topdownHUD;
class SEchoSettingsWidget;
class SEchoStatsWidget; 

class SEchoPauseWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEchoPauseWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<Acringe3_topdownHUD>, OwningHUD)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }

private:
	void OnResumeClicked();
	void OnSettingsClicked();
	void OnSettingsClosed();

	void OnStatsClicked();
	void OnStatsClosed();

	void OnExitClicked();

	TSharedRef<SWidget> CreatePauseMenuItem(const FString& ItemText, FSimpleDelegate OnClicked);

	TWeakObjectPtr<Acringe3_topdownHUD> OwningHUD;
	TSharedPtr<class SVerticalBox> MenuContainer;
	TSharedPtr<SEchoSettingsWidget> SettingsWidget;

	TSharedPtr<SEchoStatsWidget> StatsWidget;
};