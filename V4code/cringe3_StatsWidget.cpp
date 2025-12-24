#include "cringe3_StatsWidget.h"
#include "cringe3_MenuWidget.h" 
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"

namespace EchoThemeStats
{
	const FLinearColor TextNormal = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor TextHighlight = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor TextDim = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
	const FLinearColor AccentColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
	const FLinearColor DividerColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.2f);
}

void SEchoStatsWidget::Construct(const FArguments& InArgs)
{
	OnCloseDelegate = InArgs._OnClose;

	ChildSlot
		[
			SNew(SOverlay)
				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f))
				]
				+ SOverlay::Slot().Padding(40)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 20)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(STextBlock)
										.Text(FText::FromString("CHARACTER STATISTICS"))
										.Font(GetEchoFont(42))
										.ColorAndOpacity(EchoThemeStats::TextHighlight)
								]
								+ SHorizontalBox::Slot().FillWidth(1.0f)
								[
									SNew(SSpacer)
								]
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)
								[
									SAssignNew(PointsText, STextBlock)
										.Text(FText::FromString(FString::Printf(TEXT("Available Points: %d"), Stats.AvailablePoints)))
										.Font(GetEchoFont(20))
										.ColorAndOpacity(EchoThemeStats::AccentColor)
								]
						]

						+ SVerticalBox::Slot().FillHeight(1.0f)
						[
							SNew(SHorizontalBox)

								+ SHorizontalBox::Slot().FillWidth(0.3f).Padding(0, 0, 20, 0)
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
										[
											SNew(STextBlock).Text(FText::FromString("ATTRIBUTES")).Font(GetEchoFont(24)).ColorAndOpacity(EchoThemeStats::TextHighlight)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 5)
										[
											CreateAttributeRow(TEXT("Strength"), TEXT("STR"), &Stats.STR, 0.15f)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 5)
										[
											CreateAttributeRow(TEXT("Dexterity"), TEXT("DEX"), &Stats.DEX, 0.12f)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 5)
										[
											CreateAttributeRow(TEXT("Intellect"), TEXT("INT"), &Stats.INT, 0.15f)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 5)
										[
											CreateAttributeRow(TEXT("Luck"), TEXT("LUK"), &Stats.LUK, 0.20f)
										]
										+ SVerticalBox::Slot().FillHeight(1.0f)[SNew(SSpacer)]
								]

								+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0)
								[
									SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(EchoThemeStats::DividerColor).DesiredSizeOverride(FVector2D(1, 0))
								]

								+ SHorizontalBox::Slot().FillWidth(0.35f).Padding(20, 0)
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
										[
											SNew(STextBlock).Text(FText::FromString("VITALS")).Font(GetEchoFont(24)).ColorAndOpacity(EchoThemeStats::TextHighlight)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[CreateVitalRow(TEXT("Health"), TEXT("MaxHP"), HPText)]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[CreateVitalRow(TEXT("Stamina"), TEXT("STA"), StaminaText)]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[CreateVitalRow(TEXT("Barrier"), TEXT("BAR"), BarrierText)]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[CreateVitalRow(TEXT("Phys. Armor"), TEXT("ARM"), ArmorText)]

										+ SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 10)
										[
											SNew(STextBlock).Text(FText::FromString("SECONDARY")).Font(GetEchoFont(24)).ColorAndOpacity(EchoThemeStats::TextHighlight)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[CreateVitalRow(TEXT("Evasion"), TEXT("EVA"), EvasionText)]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[CreateVitalRow(TEXT("Item Drop"), TEXT("DRP"), DropRateText)]
								]

								+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0)
								[
									SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(EchoThemeStats::DividerColor).DesiredSizeOverride(FVector2D(1, 0))
								]

								+ SHorizontalBox::Slot().FillWidth(0.35f).Padding(20, 0)
								[
									SNew(SScrollBox)
										+ SScrollBox::Slot()
										[
											SNew(SVerticalBox)
												+ SVerticalBox::Slot().AutoHeight()
												[
													CreateInfoBlock(TEXT("DAMAGE TYPES"), {
														TEXT("SLASH: vs Fabric/Leather"),
														TEXT("PIERCE: vs Plate/Scale"),
														TEXT("CRUSH: vs Bone/Stone"),
														TEXT("VOID: Increases Taken Dmg (+15%)")
														})
												]
												+ SVerticalBox::Slot().AutoHeight().Padding(0, 20)
												[
													CreateInfoBlock(TEXT("ELEMENTS"), {
														TEXT("FIRE: Burn DoT"),
														TEXT("COLD: Slow Anim/Move"),
														TEXT("LIGHT: Stagger/Shock"),
														TEXT("POISON: Armor Reduction")
														})
												]
												+ SVerticalBox::Slot().AutoHeight().Padding(0, 20)
												[
													SNew(SVerticalBox)
														+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
														[
															SNew(STextBlock).Text(FText::FromString("ACTION COSTS (Stamina)")).Font(GetEchoFont(18)).ColorAndOpacity(EchoThemeStats::TextHighlight)
														]
														+ SVerticalBox::Slot().AutoHeight()[CreateActionCostRow(TEXT("Light Atk"), TEXT("10"))]
														+ SVerticalBox::Slot().AutoHeight()[CreateActionCostRow(TEXT("Heavy Atk"), TEXT("30"))]
														+ SVerticalBox::Slot().AutoHeight()[CreateActionCostRow(TEXT("Dash"), TEXT("15"))]
														+ SVerticalBox::Slot().AutoHeight()[CreateActionCostRow(TEXT("Parry"), TEXT("20"))]
														+ SVerticalBox::Slot().AutoHeight()[CreateActionCostRow(TEXT("Block"), TEXT("5/s"))]
												]
										]
								]
						]

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 0)
						[
							SNew(SButton)
								.ButtonStyle(FCoreStyle::Get(), "NoBorder")
								.OnClicked(this, &SEchoStatsWidget::OnBackClicked)
								.ContentPadding(FMargin(10, 5))
								[
									SNew(STextBlock).Text(FText::FromString("[ BACK ]")).Font(GetEchoFont(22)).ColorAndOpacity(EchoThemeStats::TextDim)
								]
						]
				]
		];

	RecalculateStats();
}

TSharedRef<SWidget> SEchoStatsWidget::CreateAttributeRow(const FString& Name, const FString& Code, int32* StatValuePtr, float GrowthFactor)
{
	TSharedPtr<STextBlock> ValWidget;
	TSharedPtr<STextBlock> CostWidget;

	TSharedRef<SWidget> Row = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.Padding(5)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock).Text(FText::FromString(Name)).Font(GetEchoFont(16)).ColorAndOpacity(EchoThemeStats::TextNormal)
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock).Text(FText::FromString(Code)).Font(GetEchoFont(10)).ColorAndOpacity(EchoThemeStats::TextDim)
						]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0).VAlign(VAlign_Center)
				[
					SAssignNew(ValWidget, STextBlock)
						.Text(FText::FromString("0"))
						.Font(GetEchoFont(20))
						.ColorAndOpacity(EchoThemeStats::TextHighlight)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0).VAlign(VAlign_Center)
				[
					SAssignNew(CostWidget, STextBlock)
						.Text(FText::FromString("Cost: 0"))
						.Font(GetEchoFont(10))
						.ColorAndOpacity(EchoThemeStats::TextDim)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0).VAlign(VAlign_Center)
				[
					SNew(SButton)
						.ButtonStyle(FCoreStyle::Get(), "NoBorder")
						.ContentPadding(FMargin(5, 0))
						.OnClicked_Lambda([this, StatValuePtr]() {
						OnIncreaseAttribute(StatValuePtr);
						return FReply::Handled();
							})
						[
							SNew(STextBlock).Text(FText::FromString("[+]")).Font(GetEchoFont(18)).ColorAndOpacity(EchoThemeStats::AccentColor)
						]
				]
		];

	StatValueWidgets.Add(StatValuePtr, ValWidget);
	StatCostWidgets.Add(StatValuePtr, CostWidget);

	return Row;
}

TSharedRef<SWidget> SEchoStatsWidget::CreateVitalRow(const FString& Name, const FString& Code, TSharedPtr<STextBlock>& OutValueText)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%s (%s)"), *Name, *Code))).Font(GetEchoFont(16)).ColorAndOpacity(EchoThemeStats::TextNormal)
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SAssignNew(OutValueText, STextBlock).Text(FText::FromString("0")).Font(GetEchoFont(16)).ColorAndOpacity(EchoThemeStats::TextHighlight)
		];
}

TSharedRef<SWidget> SEchoStatsWidget::CreateInfoBlock(const FString& Title, const TArray<FString>& Lines)
{
	TSharedPtr<SVerticalBox> VBox = SNew(SVerticalBox);

	VBox->AddSlot().AutoHeight().Padding(0, 0, 0, 5)
		[
			SNew(STextBlock).Text(FText::FromString(Title)).Font(GetEchoFont(18)).ColorAndOpacity(EchoThemeStats::TextHighlight)
		];

	for (const FString& Line : Lines)
	{
		VBox->AddSlot().AutoHeight().Padding(0, 2)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("-") + Line)).Font(GetEchoFont(12)).ColorAndOpacity(EchoThemeStats::TextNormal)
			];
	}

	return VBox.ToSharedRef();
}

TSharedRef<SWidget> SEchoStatsWidget::CreateActionCostRow(const FString& Action, const FString& Cost)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(STextBlock).Text(FText::FromString(Action)).Font(GetEchoFont(14)).ColorAndOpacity(EchoThemeStats::TextNormal)
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(STextBlock).Text(FText::FromString(Cost)).Font(GetEchoFont(14)).ColorAndOpacity(EchoThemeStats::AccentColor)
		];
}

FString SEchoStatsWidget::GetCostText(int32 CurrentValue, float GrowthFactor) const
{
	int32 BaseCost = 100;
	int32 Cost = BaseCost * (1.0f + ((float)CurrentValue * GrowthFactor));
	return FString::Printf(TEXT("Next: %d OC"), Cost);
}

void SEchoStatsWidget::OnIncreaseAttribute(int32* StatValuePtr)
{
	if (Stats.AvailablePoints > 0)
	{
		(*StatValuePtr)++;
		Stats.AvailablePoints--;
		RecalculateStats();
	}
}

void SEchoStatsWidget::RecalculateStats()
{
	if (PointsText.IsValid())
	{
		PointsText->SetText(FText::FromString(FString::Printf(TEXT("Available Points: %d"), Stats.AvailablePoints)));
	}

	auto UpdateAttrEntry = [&](int32* Ptr, float Factor) {
		if (StatValueWidgets.Contains(Ptr)) StatValueWidgets[Ptr]->SetText(FText::AsNumber(*Ptr));
		if (StatCostWidgets.Contains(Ptr)) StatCostWidgets[Ptr]->SetText(FText::FromString(GetCostText(*Ptr, Factor)));
		};

	UpdateAttrEntry(&Stats.STR, 0.15f);
	UpdateAttrEntry(&Stats.DEX, 0.12f);
	UpdateAttrEntry(&Stats.INT, 0.15f);
	UpdateAttrEntry(&Stats.LUK, 0.20f);

	float MaxHP = Stats.BaseHP + (Stats.STR * 6.0f) + (Stats.Level * 2.0f);
	if (HPText.IsValid()) HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), MaxHP)));

	float MaxStam = Stats.BaseStamina + (Stats.STR * 2.0f) + (Stats.DEX * 1.5f);
	if (StaminaText.IsValid()) StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), MaxStam)));

	float MaxBarrier = Stats.BaseBarrier + (Stats.INT * 5.0f) + (Stats.Level * 2.0f);
	if (BarrierText.IsValid()) BarrierText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), MaxBarrier)));

	float Armor = 10.0f + (Stats.STR * 0.2f);
	if (ArmorText.IsValid()) ArmorText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Armor)));

	float Evasion = 1.0f + (Stats.DEX * 0.1f) + (Stats.LUK * 0.2f);
	if (EvasionText.IsValid()) EvasionText->SetText(FText::FromString(FString::Printf(TEXT("%.1f%%"), Evasion)));

	float Drop = 100.0f + (Stats.LUK * 1.5f);
	if (DropRateText.IsValid()) DropRateText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Drop)));
}

FReply SEchoStatsWidget::OnBackClicked()
{
	if (OnCloseDelegate.IsBound())
	{
		OnCloseDelegate.Execute();
	}
	return FReply::Handled();
}