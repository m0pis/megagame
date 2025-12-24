#include "cringe3_MenuWidget.h"
#include "cringe3_MenuHUD.h"
#include "cringe3_ParticleWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Fonts/SlateFontInfo.h"
#include "cringe3_SettingsWidget.h"

namespace EchoTheme
{
	const FLinearColor TextNormal = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor TextHighlight = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor DeepBlack = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	const FLinearColor TextDim = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
	const FLinearColor TextOutlineColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	const FLinearColor GlowColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
}

FSlateFontInfo GetEchoFont(int32 Size)
{
	FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular", Size);
	Font.OutlineSettings.OutlineSize = 1.0f;
	Font.OutlineSettings.OutlineColor = EchoTheme::TextOutlineColor;
	return Font;
}

void SMainMenuWidget::Construct(const FArguments& InArgs)
{
	OwningHUD = InArgs._OwningHUD;
	bIsPressKeyScreen = true;
	TimeAccumulator = 0.0f;

	FSlateFontInfo TitleFont = GetEchoFont(140);
	FSlateFontInfo PressKeyFont = GetEchoFont(28);
	FSlateFontInfo SmallFont = GetEchoFont(12);

	ChildSlot
		[
			SNew(SOverlay)

				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.ColorAndOpacity(EchoTheme::DeepBlack)
				]

				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SParticleBackground)
				]

				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(100, 100, 0, 0)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
								.Text(FText::FromString("ECHO OF"))
								.Font(GetEchoFont(80))
								.ColorAndOpacity(EchoTheme::TextHighlight)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(5, -15, 0, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString("ORIGINAL SIN"))
								.Font(TitleFont)
								.ColorAndOpacity(EchoTheme::TextHighlight)
						]
				]

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0, 0, 0, 150)
				[
					SAssignNew(PressAnyKeyContainer, SBox)
						[
							SAssignNew(PressKeyText, STextBlock)
								.Text(FText::FromString("Press Any Key"))
								.Font(PressKeyFont)
								.ColorAndOpacity(EchoTheme::TextNormal)
						]
				]

				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(120, 150, 0, 0)
				[
					SAssignNew(MainMenuContainer, SVerticalBox)
						.Visibility(EVisibility::Hidden)

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreateMenuItem(TEXT("Continue"), FSimpleDelegate())]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreateMenuItem(TEXT("New Game"), FSimpleDelegate::CreateSP(this, &SMainMenuWidget::OnNewGameClicked))]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreateMenuItem(TEXT("Load Game"), FSimpleDelegate())]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreateMenuItem(TEXT("Settings"), FSimpleDelegate::CreateSP(this, &SMainMenuWidget::OnSettingsClicked))]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[CreateMenuItem(TEXT("Exit"), FSimpleDelegate::CreateSP(this, &SMainMenuWidget::OnQuitClicked))]
				]

				+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(0, 0, 50, 30)
				[
					SNew(STextBlock)
						.Text(FText::FromString("ZOV STUDIOS (C) 2025"))
						.Font(SmallFont)
						.ColorAndOpacity(EchoTheme::TextDim)
				]

				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SAssignNew(SettingsWidget, SEchoSettingsWidget)
						.OnClose(FSimpleDelegate::CreateSP(this, &SMainMenuWidget::OnSettingsClosed))
						.Visibility(EVisibility::Collapsed)
				]
		];
}

TSharedRef<SWidget> SMainMenuWidget::CreateMenuItem(const FString& ItemText, FSimpleDelegate OnClicked)
{
	return SNew(SEchoMenuItem).ItemText(ItemText).OnClicked(OnClicked);
}

void SMainMenuWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bIsPressKeyScreen)
	{
		TimeAccumulator += InDeltaTime;
		float Alpha = (FMath::Sin(TimeAccumulator * 3.0f) + 1.0f) * 0.5f;
		float Opacity = 0.2f + (Alpha * 0.8f);
		if (PressKeyText.IsValid()) PressKeyText->SetColorAndOpacity(EchoTheme::TextHighlight.CopyWithNewOpacity(Opacity));
	}
}
FReply SMainMenuWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) { ProceedToMenu(); return FReply::Handled(); }
FReply SMainMenuWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) { ProceedToMenu(); return FReply::Handled(); }
void SMainMenuWidget::ProceedToMenu()
{
	if (bIsPressKeyScreen)
	{
		bIsPressKeyScreen = false;
		if (PressAnyKeyContainer.IsValid()) PressAnyKeyContainer->SetVisibility(EVisibility::Collapsed);
		if (MainMenuContainer.IsValid()) MainMenuContainer->SetVisibility(EVisibility::Visible);
	}
}
void SMainMenuWidget::OnNewGameClicked() { if (OwningHUD.IsValid()) OwningHUD->PlayGame(); }
void SMainMenuWidget::OnQuitClicked() { if (OwningHUD.IsValid()) OwningHUD->QuitGame(); }

void SEchoMenuItem::Construct(const FArguments& InArgs)
{
	OnClickedDelegate = InArgs._OnClicked;
	bIsHovered = false;
	GlowAccumulator = 0.0f;
	FSlateFontInfo ItemFont = GetEchoFont(36);

	ChildSlot
		[
			SNew(SOverlay)
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(500.0f).HeightOverride(60.0f)
						[
							SAssignNew(GlowBackground, SImage)
								.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.ColorAndOpacity(EchoTheme::GlowColor.CopyWithNewOpacity(0.0f))
						]
				]
				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 15, 0).VAlign(VAlign_Center)
						[
							SAssignNew(BarBlock, STextBlock).Text(FText::FromString("|")).Font(ItemFont).ColorAndOpacity(EchoTheme::TextHighlight).Visibility(EVisibility::Hidden)
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SAssignNew(TextBlock, STextBlock).Text(FText::FromString(InArgs._ItemText)).Font(ItemFont).ColorAndOpacity(EchoTheme::TextNormal)
						]
				]
		];
}

void SEchoMenuItem::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bIsHovered)
	{
		GlowAccumulator += InDeltaTime;
		float Pulse = (FMath::Sin(GlowAccumulator * 8.0f) + 1.0f) * 0.5f;
		float GlowOpacity = 0.05f + (Pulse * 0.1f);
		if (GlowBackground.IsValid()) GlowBackground->SetColorAndOpacity(EchoTheme::GlowColor.CopyWithNewOpacity(GlowOpacity));
	}
}
void SEchoMenuItem::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	bIsHovered = true;
	if (BarBlock.IsValid()) BarBlock->SetVisibility(EVisibility::Visible);
	if (GlowBackground.IsValid()) GlowBackground->SetVisibility(EVisibility::Visible);
	if (TextBlock.IsValid()) TextBlock->SetColorAndOpacity(EchoTheme::TextHighlight);
}
void SEchoMenuItem::OnMouseLeave(const FPointerEvent& MouseEvent) {
	bIsHovered = false;
	if (BarBlock.IsValid()) BarBlock->SetVisibility(EVisibility::Hidden);
	if (GlowBackground.IsValid()) GlowBackground->SetVisibility(EVisibility::Hidden);
	if (TextBlock.IsValid()) TextBlock->SetColorAndOpacity(EchoTheme::TextNormal);
}
void SMainMenuWidget::OnSettingsClicked()
{
	if (MainMenuContainer.IsValid()) MainMenuContainer->SetVisibility(EVisibility::Collapsed);
	if (SettingsWidget.IsValid()) SettingsWidget->SetVisibility(EVisibility::Visible);
}

void SMainMenuWidget::OnSettingsClosed()
{
	if (SettingsWidget.IsValid()) SettingsWidget->SetVisibility(EVisibility::Collapsed);
	if (MainMenuContainer.IsValid()) MainMenuContainer->SetVisibility(EVisibility::Visible);
}
FReply SEchoMenuItem::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) { if (OnClickedDelegate.IsBound()) OnClickedDelegate.Execute(); return FReply::Handled(); }