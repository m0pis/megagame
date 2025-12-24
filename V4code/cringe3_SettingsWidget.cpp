#include "cringe3_SettingsWidget.h"
#include "cringe3_MenuWidget.h"
#include "cringe3_ParticleWidget.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SScrollBox.h" 
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ConfigCacheIni.h"

namespace EchoThemeSettings
{
	const FLinearColor TextNormal = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor TextHighlight = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor DeepBlack = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	const FLinearColor TextDim = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

	const FLinearColor GlowColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
	const FLinearColor SliderFill = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
}

class SEchoSettingRow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEchoSettingRow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FEchoSettingItemData> InData, SEchoSettingsWidget* InOwner)
	{
		Data = InData;
		OwnerWidget = InOwner;

		TSharedPtr<SWidget> ValueWidget;

		if (Data->Type == ESettingType::List)
		{
			ValueWidget = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					CreateArrowButton(TEXT("<"), -1)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0)
				[
					SAssignNew(ValueText, STextBlock)
						.Text(FText::FromString(Data->Options.IsValidIndex(Data->CurrentOptionIndex) ? Data->Options[Data->CurrentOptionIndex] : TEXT("ERR")))
						.Font(GetEchoFont(16))
						.ColorAndOpacity(EchoThemeSettings::TextHighlight)
						.MinDesiredWidth(150.0f)
						.Justification(ETextJustify::Center)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					CreateArrowButton(TEXT(">"), 1)
				];
		}
		else
		{
			ValueWidget = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 15, 0).VAlign(VAlign_Center)
				[
					SAssignNew(ValueText, STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%.0f"), Data->CurrentValue)))
						.Font(GetEchoFont(16))
						.ColorAndOpacity(EchoThemeSettings::TextHighlight)
						.MinDesiredWidth(40.0f)
						.Justification(ETextJustify::Right)
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(200.0f)
						[
							SAssignNew(SliderWidget, SSlider)
								.Value(GetSliderNormalizedValue())
								.OnValueChanged(this, &SEchoSettingRow::OnSliderChanged)
								.Style(&FCoreStyle::Get().GetWidgetStyle<FSliderStyle>("Slider"))
								.Orientation(Orient_Horizontal)
								.SliderBarColor(EchoThemeSettings::SliderFill)
						]
				];
		}

		ChildSlot
			[
				SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ContentPadding(FMargin(15, 12))
					.OnHovered_Lambda([this]() { if (OwnerWidget) OwnerWidget->OnSettingHovered(Data); })
					.OnUnhovered_Lambda([this]() { if (OwnerWidget) OwnerWidget->OnSettingUnhovered(); })
					[
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString(Data->Name))
									.Font(GetEchoFont(18))
									.ColorAndOpacity(EchoThemeSettings::TextNormal)
							]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								ValueWidget.ToSharedRef()
							]
					]
			];
	}

private:
	TSharedRef<SWidget> CreateArrowButton(const FString& Text, int32 Dir)
	{
		return SNew(SButton)
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			.ContentPadding(FMargin(5, 0))
			.OnClicked_Lambda([this, Dir]() { ChangeOption(Dir); return FReply::Handled(); })
			[
				SNew(STextBlock).Text(FText::FromString(Text)).Font(GetEchoFont(18)).ColorAndOpacity(EchoThemeSettings::TextDim)
			];
	}

	void ChangeOption(int32 Dir)
	{
		Data->CurrentOptionIndex += Dir;
		if (Data->CurrentOptionIndex < 0) Data->CurrentOptionIndex = Data->Options.Num() - 1;
		if (Data->CurrentOptionIndex >= Data->Options.Num()) Data->CurrentOptionIndex = 0;

		if (ValueText.IsValid()) ValueText->SetText(FText::FromString(Data->Options[Data->CurrentOptionIndex]));
	}

	float GetSliderNormalizedValue() const
	{
		if (Data->MaxValue - Data->MinValue == 0.0f) return 0.0f;
		return (Data->CurrentValue - Data->MinValue) / (Data->MaxValue - Data->MinValue);
	}

	void OnSliderChanged(float NewNormalizedValue)
	{
		float NewVal = Data->MinValue + (NewNormalizedValue * (Data->MaxValue - Data->MinValue));
		if (Data->Step > 0) NewVal = FMath::RoundToFloat(NewVal / Data->Step) * Data->Step;

		Data->CurrentValue = NewVal;
		FString StrVal = (Data->Step < 1.0f) ? FString::Printf(TEXT("%.1f"), NewVal) : FString::Printf(TEXT("%.0f"), NewVal);
		if (ValueText.IsValid()) ValueText->SetText(FText::FromString(StrVal));
	}

	TSharedPtr<FEchoSettingItemData> Data;
	SEchoSettingsWidget* OwnerWidget;
	TSharedPtr<STextBlock> ValueText;
	TSharedPtr<SSlider> SliderWidget;
};

void SEchoSettingsWidget::Construct(const FArguments& InArgs)
{
	OnCloseDelegate = InArgs._OnClose;
	CurrentTab = ESettingsTab::Video;

	InitializeSettings();

	ChildSlot
		[
			SNew(SOverlay)
				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.ColorAndOpacity(EchoThemeSettings::DeepBlack)
				]
				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SParticleBackground)
				]
				+ SOverlay::Slot().Padding(60.0f)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 30)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock).Text(FText::FromString("SETTINGS")).Font(GetEchoFont(48)).ColorAndOpacity(EchoThemeSettings::TextHighlight)
								]
								+ SHorizontalBox::Slot().FillWidth(1.0f)[SNew(SSpacer)]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									BuildTabs()
								]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 20)
						[
							SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(EchoThemeSettings::TextDim).DesiredSizeOverride(FVector2D(0, 1))
						]
						+ SVerticalBox::Slot().FillHeight(1.0f)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().FillWidth(0.55f).Padding(0, 0, 40, 0)
								[
									SNew(SScrollBox)
										+ SScrollBox::Slot()
										[
											SAssignNew(SettingsListContainer, SVerticalBox)
										]
								]
								+ SHorizontalBox::Slot().FillWidth(0.45f).Padding(40, 50, 0, 0).VAlign(VAlign_Top)
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight()
										[
											SAssignNew(DescriptionTitle, STextBlock)
												.Text(FText::FromString("Welcome"))
												.Font(GetEchoFont(28))
												.ColorAndOpacity(EchoThemeSettings::GlowColor)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 10)
										[
											SNew(SBox).HeightOverride(2).WidthOverride(100).HAlign(HAlign_Left)
												[
													SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(EchoThemeSettings::TextHighlight)
												]
										]
										+ SVerticalBox::Slot().AutoHeight()
										[
											SAssignNew(DescriptionBody, STextBlock)
												.Text(FText::FromString("Hover over an option to see details."))
												.Font(GetEchoFont(16))
												.ColorAndOpacity(EchoThemeSettings::TextNormal)
												.AutoWrapText(true)
										]
								]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 0).HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 30, 0)
								[
									SNew(SButton)
										.ButtonStyle(FCoreStyle::Get(), "NoBorder")
										.OnClicked(this, &SEchoSettingsWidget::OnBackClicked)
										.ContentPadding(FMargin(10, 5))
										[
											SNew(STextBlock).Text(FText::FromString("[ BACK ]")).Font(GetEchoFont(22)).ColorAndOpacity(EchoThemeSettings::TextDim)
										]
								]
								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(SButton)
										.ButtonStyle(FCoreStyle::Get(), "NoBorder")
										.OnClicked(this, &SEchoSettingsWidget::OnApplyClicked)
										.ContentPadding(FMargin(10, 5))
										[
											SNew(STextBlock).Text(FText::FromString("[ APPLY ]")).Font(GetEchoFont(22)).ColorAndOpacity(EchoThemeSettings::TextHighlight)
										]
								]
						]
				]
		];

	RebuildSettingsList();
}

UGameUserSettings* SEchoSettingsWidget::GetGUS() const
{
	if (GEngine) return GEngine->GetGameUserSettings();
	return nullptr;
}

void SEchoSettingsWidget::InitializeSettings()
{
	InitVideoSettings();
	InitGraphicsSettings();
	InitAudioSettings();   
	InitGameplaySettings();
}

void SEchoSettingsWidget::InitVideoSettings()
{
	TArray<TSharedPtr<FEchoSettingItemData>> Items;
	UGameUserSettings* GUS = GetGUS();

	TArray<FString> WinModes = { TEXT("Fullscreen"), TEXT("Windowed Fullscreen"), TEXT("Windowed") };
	int32 CurrentMode = 1;
	if (GUS) {
		EWindowMode::Type Mode = GUS->GetFullscreenMode();
		if (Mode == EWindowMode::Fullscreen) CurrentMode = 0;
		else if (Mode == EWindowMode::WindowedFullscreen) CurrentMode = 1;
		else CurrentMode = 2;
	}
	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Window Mode"), TEXT("Select how the game window is displayed."), WinModes, CurrentMode,
		FOnIntSettingChanged::CreateLambda([this](int32 Val) {
			if (auto S = GetGUS()) {
				if (Val == 0) S->SetFullscreenMode(EWindowMode::Fullscreen);
				else if (Val == 1) S->SetFullscreenMode(EWindowMode::WindowedFullscreen);
				else S->SetFullscreenMode(EWindowMode::Windowed);
			}
			})));

	TArray<FString> ResOpts = { TEXT("1920x1080"), TEXT("2560x1440"), TEXT("3840x2160") };
	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Resolution"), TEXT("Screen resolution."), ResOpts, 0,
		FOnIntSettingChanged::CreateLambda([this](int32 Val) {
			if (auto S = GetGUS()) {
				FIntPoint P(1920, 1080);
				if (Val == 1) P = FIntPoint(2560, 1440);
				if (Val == 2) P = FIntPoint(3840, 2160);
				S->SetScreenResolution(P);
			}
			})));

	bool bVSync = GUS ? GUS->IsVSyncEnabled() : false;
	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("VSync"), TEXT("Sync frame rate to monitor refresh rate."), TArray<FString>{TEXT("Off"), TEXT("On")}, bVSync ? 1 : 0,
		FOnIntSettingChanged::CreateLambda([this](int32 Val) {
			if (auto S = GetGUS()) { S->SetVSyncEnabled(Val == 1); }
			})));

	float CurrentFPS = GUS ? GUS->GetFrameRateLimit() : 0.0f;
	if (CurrentFPS <= 0.0f) CurrentFPS = 300.0f;
	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Frame Rate Cap"), TEXT("Limit maximum FPS to save power."), 30.0f, 300.0f, CurrentFPS, 10.0f,
		FOnFloatSettingChanged::CreateLambda([this](float Val) {
			if (auto S = GetGUS()) { S->SetFrameRateLimit(Val >= 290.0f ? 0.0f : Val); }
			})));

	float CurrentScale = 100.0f;
	if (GUS) {
		float CurrentScaleNormalized = 0.0f; float CurrentScaleValue = 0.0f;
		float MinScaleValue = 0.0f; float MaxScaleValue = 0.0f;
		GUS->GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScaleValue, MinScaleValue, MaxScaleValue);
		if (CurrentScaleValue > 0.0f) CurrentScale = CurrentScaleValue;
	}
	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Resolution Scale"), TEXT("Internal rendering resolution."), 50.0f, 100.0f, CurrentScale, 1.0f,
		FOnFloatSettingChanged::CreateLambda([this](float Val) {
			if (auto S = GetGUS()) { S->SetResolutionScaleValueEx(Val); }
			})));

	SettingsMap.Add(ESettingsTab::Video, Items);
}

void SEchoSettingsWidget::InitGraphicsSettings()
{
	TArray<TSharedPtr<FEchoSettingItemData>> Items;
	UGameUserSettings* GUS = GetGUS();

	TArray<FString> QualityOpts = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic") };

	auto MakeQuality = [&](FString Name, FString Desc, int32(UGameUserSettings::* Get)() const, void (UGameUserSettings::* Set)(int32)) {
		int32 Val = GUS ? (GUS->*Get)() : 1;
		Items.Add(MakeShared<FEchoSettingItemData>(Name, Desc, QualityOpts, Val,
			FOnIntSettingChanged::CreateLambda([this, Set](int32 V) {
				if (auto S = GetGUS()) { (S->*Set)(V); }
				})));
		};

	MakeQuality(TEXT("View Distance"), TEXT("How far objects are rendered."), &UGameUserSettings::GetViewDistanceQuality, &UGameUserSettings::SetViewDistanceQuality);
	MakeQuality(TEXT("Anti-Aliasing"), TEXT("Smoothes jagged edges."), &UGameUserSettings::GetAntiAliasingQuality, &UGameUserSettings::SetAntiAliasingQuality);
	MakeQuality(TEXT("Post Processing"), TEXT("Quality of bloom, motion blur, and color grading."), &UGameUserSettings::GetPostProcessingQuality, &UGameUserSettings::SetPostProcessingQuality);
	MakeQuality(TEXT("Shadows"), TEXT("Resolution and distance of shadows."), &UGameUserSettings::GetShadowQuality, &UGameUserSettings::SetShadowQuality);
	MakeQuality(TEXT("Global Illumination"), TEXT("Lighting quality (Lumen)."), &UGameUserSettings::GetGlobalIlluminationQuality, &UGameUserSettings::SetGlobalIlluminationQuality);
	MakeQuality(TEXT("Reflections"), TEXT("Quality of surface reflections."), &UGameUserSettings::GetReflectionQuality, &UGameUserSettings::SetReflectionQuality);
	MakeQuality(TEXT("Textures"), TEXT("Resolution of textures."), &UGameUserSettings::GetTextureQuality, &UGameUserSettings::SetTextureQuality);
	MakeQuality(TEXT("Effects"), TEXT("Quality of particles and VFX."), &UGameUserSettings::GetVisualEffectQuality, &UGameUserSettings::SetVisualEffectQuality);

	SettingsMap.Add(ESettingsTab::AdvancedGraphics, Items);
}

void SEchoSettingsWidget::InitAudioSettings()
{
	TArray<TSharedPtr<FEchoSettingItemData>> Items;

	float MasterVol = 100.0f; GConfig->GetFloat(TEXT("Audio"), TEXT("MasterVolume"), MasterVol, GGameIni);
	float MusicVol = 80.0f;   GConfig->GetFloat(TEXT("Audio"), TEXT("MusicVolume"), MusicVol, GGameIni);
	float SFXVol = 100.0f;    GConfig->GetFloat(TEXT("Audio"), TEXT("SFXVolume"), SFXVol, GGameIni);

	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Master Volume"), TEXT("Global game volume."), 0.0f, 100.0f, MasterVol, 1.0f,
		FOnFloatSettingChanged::CreateLambda([](float Val) {
			GConfig->SetFloat(TEXT("Audio"), TEXT("MasterVolume"), Val, GGameIni);
			})));

	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Music Volume"), TEXT("Background score volume."), 0.0f, 100.0f, MusicVol, 1.0f,
		FOnFloatSettingChanged::CreateLambda([](float Val) {
			GConfig->SetFloat(TEXT("Audio"), TEXT("MusicVolume"), Val, GGameIni);
			})));

	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("SFX Volume"), TEXT("Sound effects volume."), 0.0f, 100.0f, SFXVol, 1.0f,
		FOnFloatSettingChanged::CreateLambda([](float Val) {
			GConfig->SetFloat(TEXT("Audio"), TEXT("SFXVolume"), Val, GGameIni);
			})));

	SettingsMap.Add(ESettingsTab::Audio, Items);
}

void SEchoSettingsWidget::InitGameplaySettings()
{
	TArray<TSharedPtr<FEchoSettingItemData>> Items;

	float Sensitivity = 1.0f;
	GConfig->GetFloat(TEXT("Gameplay"), TEXT("MouseSensitivity"), Sensitivity, GGameIni);

	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Camera Sensitivity"), TEXT("Mouse rotation speed."), 0.1f, 5.0f, Sensitivity, 0.1f,
		FOnFloatSettingChanged::CreateLambda([](float Val) {
			GConfig->SetFloat(TEXT("Gameplay"), TEXT("MouseSensitivity"), Val, GGameIni);
			})));

	bool bInvertY = false;
	GConfig->GetBool(TEXT("Gameplay"), TEXT("InvertY"), bInvertY, GGameIni);
	TArray<FString> BoolOpts = { TEXT("Off"), TEXT("On") };
	Items.Add(MakeShared<FEchoSettingItemData>(TEXT("Invert Y-Axis"), TEXT("Invert vertical mouse control."), BoolOpts, bInvertY ? 1 : 0,
		FOnIntSettingChanged::CreateLambda([](int32 Val) {
			GConfig->SetBool(TEXT("Gameplay"), TEXT("InvertY"), Val == 1, GGameIni);
			})));

	SettingsMap.Add(ESettingsTab::Gameplay, Items);
}

TSharedRef<SWidget> SEchoSettingsWidget::BuildTabs()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(20, 0)[BuildTabButton(ESettingsTab::Video, "VIDEO")]
		+ SHorizontalBox::Slot().AutoWidth().Padding(20, 0)[BuildTabButton(ESettingsTab::AdvancedGraphics, "GRAPHICS")]
		+ SHorizontalBox::Slot().AutoWidth().Padding(20, 0)[BuildTabButton(ESettingsTab::Audio, "AUDIO")]      
		+ SHorizontalBox::Slot().AutoWidth().Padding(20, 0)[BuildTabButton(ESettingsTab::Gameplay, "GAMEPLAY")]; 
}

TSharedRef<SWidget> SEchoSettingsWidget::BuildTabButton(ESettingsTab TabID, const FString& Text)
{
	TSharedPtr<STextBlock> NewTextWidget;
	TSharedRef<SButton> Btn = SNew(SButton)
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.OnClicked_Lambda([this, TabID]() { OnTabClicked(TabID); return FReply::Handled(); })
		.ContentPadding(FMargin(10, 5))
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(NewTextWidget, STextBlock)
						.Text(FText::FromString(Text))
						.Font(GetEchoFont(20))
						.ColorAndOpacity(TabID == CurrentTab ? EchoThemeSettings::TextHighlight : EchoThemeSettings::TextDim)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)
				[
					SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.ColorAndOpacity(TAttribute<FSlateColor>::Create([this, TabID]() {
						return (TabID == CurrentTab) ? EchoThemeSettings::GlowColor : FLinearColor::Transparent;
							}))
						.DesiredSizeOverride(FVector2D(0, 2))
				]
		];

	TabTextWidgets.Add(TabID, NewTextWidget);
	return Btn;
}

void SEchoSettingsWidget::OnTabClicked(ESettingsTab NewTab)
{
	if (CurrentTab != NewTab)
	{
		CurrentTab = NewTab;
		for (auto& Elem : TabTextWidgets)
		{
			if (Elem.Value.IsValid())
			{
				Elem.Value->SetColorAndOpacity(Elem.Key == CurrentTab ? EchoThemeSettings::TextHighlight : EchoThemeSettings::TextDim);
			}
		}
		RebuildSettingsList();
	}
}

void SEchoSettingsWidget::RebuildSettingsList()
{
	SettingsListContainer->ClearChildren();
	if (SettingsMap.Contains(CurrentTab))
	{
		TArray<TSharedPtr<FEchoSettingItemData>>& Items = SettingsMap[CurrentTab];
		for (auto& Item : Items)
		{
			SettingsListContainer->AddSlot()
				.AutoHeight()
				.Padding(0, 10)
				[
					CreateSettingRow(Item)
				];
		}
	}
}

TSharedRef<SWidget> SEchoSettingsWidget::CreateSettingRow(const TSharedPtr<FEchoSettingItemData>& ItemData)
{
	return SNew(SEchoSettingRow, ItemData, this);
}

void SEchoSettingsWidget::OnSettingHovered(TSharedPtr<FEchoSettingItemData> ItemData)
{
	if (DescriptionTitle.IsValid()) DescriptionTitle->SetText(FText::FromString(ItemData->Name));
	if (DescriptionBody.IsValid()) DescriptionBody->SetText(FText::FromString(ItemData->Description));
}

void SEchoSettingsWidget::OnSettingUnhovered() {}

FReply SEchoSettingsWidget::OnApplyClicked()
{
	for (auto& Elem : SettingsMap)
	{
		for (auto& Item : Elem.Value)
		{
			if (Item->Type == ESettingType::List)
			{
				if (Item->OnOptionChanged.IsBound())
					Item->OnOptionChanged.Execute(Item->CurrentOptionIndex);
			}
			else if (Item->Type == ESettingType::Slider)
			{
				if (Item->OnSliderChanged.IsBound())
					Item->OnSliderChanged.Execute(Item->CurrentValue);
			}
		}
	}

	if (auto S = GetGUS())
	{
		S->ApplySettings(false);
		S->SaveSettings();
	}

	GConfig->Flush(false, GGameIni);

	return FReply::Handled();
}

FReply SEchoSettingsWidget::OnBackClicked()
{
	if (OnCloseDelegate.IsBound())
	{
		OnCloseDelegate.Execute();
	}
	return FReply::Handled();
}