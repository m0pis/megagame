#include "cringe3_LoadingWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScaleBox.h"
#include "cringe3_ParticleWidget.h" 

namespace LoadingTheme
{
	const FLinearColor TextWhite = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
	const FLinearColor TextGrey = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor DeepBlack = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	const FLinearColor DividerColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
}

void SLoadingWidget::Construct(const FArguments& InArgs)
{
	RotationAccumulator = 0.0f;
	PulseAccumulator = 0.0f;

	FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Regular", 24);
	FSlateFontInfo BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 14);

	FString LoreTitle = "Rarikoak's Binding";
	FString LoreDesc = "Princess Hannah's reflection in the obsidian mirror reveals not one face, but many. It is said that Rarikoak's strength requires a vessel already broken, for only the cracks can let the true power seep in.";

	ChildSlot
		[
			SNew(SOverlay)

				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(LoadingTheme::DeepBlack)
				]

				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SParticleBackground)
				]

				+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(100, 0, 0, 100)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock).Text(FText::FromString(LoreTitle)).Font(TitleFont).ColorAndOpacity(LoadingTheme::TextWhite)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 10)
						[
							SNew(SBox).WidthOverride(300).HeightOverride(1).HAlign(HAlign_Left)
								[
									SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(LoadingTheme::DividerColor)
								]
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox).WidthOverride(600)
								[
									SNew(STextBlock).Text(FText::FromString(LoreDesc)).Font(BodyFont).ColorAndOpacity(LoadingTheme::TextGrey).AutoWrapText(true)
								]
						]
				]

				+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(0, 0, 60, 60)
				[
					SNew(SBox).WidthOverride(64).HeightOverride(64)
						[
							SNew(SOverlay)
								+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
								[
									SAssignNew(OuterRing, SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(LoadingTheme::TextWhite.CopyWithNewOpacity(0.8f))
								]
								+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
								[
									SNew(SBox).WidthOverride(32).HeightOverride(32)
										[
											SAssignNew(InnerRing, SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(LoadingTheme::TextGrey)
										]
								]
						]
				]
		];
}

void SLoadingWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	RotationAccumulator += InDeltaTime;
	PulseAccumulator += InDeltaTime;
	float PulseScale = 1.0f + (FMath::Sin(PulseAccumulator * 4.0f) * 0.15f);

	if (OuterRing.IsValid())
	{
		float Angle = RotationAccumulator * 90.0f;
		OuterRing->SetRenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(Angle))));
		OuterRing->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	}
	if (InnerRing.IsValid())
	{
		float Angle = RotationAccumulator * -180.0f;
		FTransform2D Rot(FQuat2D(FMath::DegreesToRadians(Angle)));
		FTransform2D Scale(FVector2D(PulseScale, PulseScale));
		InnerRing->SetRenderTransform(FSlateRenderTransform(Scale.Concatenate(Rot).GetMatrix()));
		InnerRing->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	}
}