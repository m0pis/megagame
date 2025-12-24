#include "cringe3_PauseWidget.h"
#include "cringe3_topdownHUD.h"
#include "cringe3_SettingsWidget.h"
#include "cringe3_StatsWidget.h" 
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSpacer.h"
#include "Fonts/SlateFontInfo.h"
#include "Kismet/GameplayStatics.h"

namespace EchoThemePause
{
	const FLinearColor TextNormal = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor TextHighlight = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor SemiTransparentBlack = FLinearColor(0.0f, 0.0f, 0.0f, 0.85f);
	const FLinearColor GlowColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
	const FLinearColor TextOutlineColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

FSlateFontInfo GetPauseEchoFont(int32 Size)
{
	FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular", Size);
	Font.OutlineSettings.OutlineSize = 1.0f;
	Font.OutlineSettings.OutlineColor = EchoThemePause::TextOutlineColor;
	return Font;
}

class SEchoPauseMenuItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEchoPauseMenuItem) : _ItemText(TEXT("Item")), _OnClicked() {}
		SLATE_ARGUMENT(FString, ItemText)
		SLATE_EVENT(FSimpleDelegate, OnClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		OnClickedDelegate = InArgs._OnClicked;
		bIsHovered = false;
		GlowAccumulator = 0.0f;
		FSlateFontInfo ItemFont = GetPauseEchoFont(36);

		ChildSlot
			[
				SNew(SBox).HeightOverride(60.0f)
					[
						SNew(SOverlay)
							+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(SBox).WidthOverride(500.0f).HeightOverride(60.0f)
									[
										SAssignNew(GlowBackground, SImage)
											.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.ColorAndOpacity(EchoThemePause::GlowColor.CopyWithNewOpacity(0.0f))
									]
							]
							+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 15, 0).VAlign(VAlign_Center)
									[
										SAssignNew(BarBlock, STextBlock)
											.Text(FText::FromString("|"))
											.Font(ItemFont)
											.ColorAndOpacity(EchoThemePause::TextHighlight)
											.Visibility(EVisibility::Hidden)
									]
									+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
									[
										SAssignNew(TextBlock, STextBlock)
											.Text(FText::FromString(InArgs._ItemText))
											.Font(ItemFont)
											.ColorAndOpacity(EchoThemePause::TextNormal)
									]
							]
					]
			];
	}

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		if (bIsHovered)
		{
			GlowAccumulator += InDeltaTime;
			float Pulse = (FMath::Sin(GlowAccumulator * 8.0f) + 1.0f) * 0.5f;
			float GlowOpacity = 0.05f + (Pulse * 0.1f);
			if (GlowBackground.IsValid()) GlowBackground->SetColorAndOpacity(EchoThemePause::GlowColor.CopyWithNewOpacity(GlowOpacity));
		}
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (OnClickedDelegate.IsBound()) OnClickedDelegate.Execute();
		return FReply::Handled();
	}

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		bIsHovered = true;
		if (BarBlock.IsValid()) BarBlock->SetVisibility(EVisibility::Visible);
		if (GlowBackground.IsValid()) GlowBackground->SetVisibility(EVisibility::Visible);
		if (TextBlock.IsValid()) TextBlock->SetColorAndOpacity(EchoThemePause::TextHighlight);
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		bIsHovered = false;
		if (BarBlock.IsValid()) BarBlock->SetVisibility(EVisibility::Hidden);
		if (GlowBackground.IsValid()) GlowBackground->SetVisibility(EVisibility::Hidden);
		if (TextBlock.IsValid()) TextBlock->SetColorAndOpacity(EchoThemePause::TextNormal);
	}

private:
	TSharedPtr<STextBlock> TextBlock;
	TSharedPtr<STextBlock> BarBlock;
	TSharedPtr<SImage> GlowBackground;
	FSimpleDelegate OnClickedDelegate;
	bool bIsHovered;
	float GlowAccumulator;
};

void SEchoPauseWidget::Construct(const FArguments& InArgs)
{
	OwningHUD = InArgs._OwningHUD;
	bCanSupportFocus = true;

	ChildSlot
		[
			SNew(SOverlay)

				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.ColorAndOpacity(EchoThemePause::SemiTransparentBlack)
				]

				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(100, 100, 0, 0)
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
								.Text(FText::FromString("PAUSED"))
								.Font(GetPauseEchoFont(80))
								.ColorAndOpacity(EchoThemePause::TextHighlight)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(5, -15, 0, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString("GAME FROZEN"))
								.Font(GetPauseEchoFont(20))
								.ColorAndOpacity(EchoThemePause::TextNormal)
						]
				]

				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(120, 100, 0, 0)
				[
					SAssignNew(MenuContainer, SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreatePauseMenuItem(TEXT("Resume"), FSimpleDelegate::CreateSP(this, &SEchoPauseWidget::OnResumeClicked))]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreatePauseMenuItem(TEXT("Statistics"), FSimpleDelegate::CreateSP(this, &SEchoPauseWidget::OnStatsClicked))] // <--- NEW BUTTON
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreatePauseMenuItem(TEXT("Settings"), FSimpleDelegate::CreateSP(this, &SEchoPauseWidget::OnSettingsClicked))]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreatePauseMenuItem(TEXT("Exit to Menu"), FSimpleDelegate::CreateSP(this, &SEchoPauseWidget::OnExitClicked))]
				]

				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SAssignNew(SettingsWidget, SEchoSettingsWidget)
						.OnClose(FSimpleDelegate::CreateSP(this, &SEchoPauseWidget::OnSettingsClosed))
						.Visibility(EVisibility::Collapsed)
				]
				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SAssignNew(StatsWidget, SEchoStatsWidget)
						.OnClose(FSimpleDelegate::CreateSP(this, &SEchoPauseWidget::OnStatsClosed))
						.Visibility(EVisibility::Collapsed)
				]
		];
}

FReply SEchoPauseWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled().SetUserFocus(SharedThis(this));
}

FReply SEchoPauseWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (SettingsWidget.IsValid() && SettingsWidget->GetVisibility() == EVisibility::Visible)
		{
			OnSettingsClosed();
			return FReply::Handled();
		}
		// --- HANDLE STATS CLOSING ON ESCAPE ---
		else if (StatsWidget.IsValid() && StatsWidget->GetVisibility() == EVisibility::Visible)
		{
			OnStatsClosed();
			return FReply::Handled();
		}
		else
		{
			OnResumeClicked();
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

TSharedRef<SWidget> SEchoPauseWidget::CreatePauseMenuItem(const FString& ItemText, FSimpleDelegate OnClicked)
{
	return SNew(SEchoPauseMenuItem).ItemText(ItemText).OnClicked(OnClicked);
}

void SEchoPauseWidget::OnResumeClicked()
{
	if (OwningHUD.IsValid())
	{
		OwningHUD->TogglePauseMenu();
	}
}

void SEchoPauseWidget::OnSettingsClicked()
{
	if (MenuContainer.IsValid()) MenuContainer->SetVisibility(EVisibility::Hidden);
	if (SettingsWidget.IsValid()) SettingsWidget->SetVisibility(EVisibility::Visible);
}

void SEchoPauseWidget::OnSettingsClosed()
{
	if (SettingsWidget.IsValid()) SettingsWidget->SetVisibility(EVisibility::Collapsed);
	if (MenuContainer.IsValid()) MenuContainer->SetVisibility(EVisibility::Visible);
}

void SEchoPauseWidget::OnExitClicked()
{
	if (OwningHUD.IsValid())
	{
		UGameplayStatics::SetGamePaused(OwningHUD.Get(), false);
		UGameplayStatics::OpenLevel(OwningHUD.Get(), FName("MainMenuMap"));
	}
}

void SEchoPauseWidget::OnStatsClicked()
{
	if (MenuContainer.IsValid()) MenuContainer->SetVisibility(EVisibility::Hidden);
	if (StatsWidget.IsValid()) StatsWidget->SetVisibility(EVisibility::Visible);
}

void SEchoPauseWidget::OnStatsClosed()
{
	if (StatsWidget.IsValid()) StatsWidget->SetVisibility(EVisibility::Collapsed);
	if (MenuContainer.IsValid()) MenuContainer->SetVisibility(EVisibility::Visible);
}