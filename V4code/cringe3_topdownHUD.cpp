#include "cringe3_topdownHUD.h"
#include "cringe3_topdownCharacter.h"
#include "cringe3_PauseWidget.h" 
#include "Engine/GameEngine.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElements.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/WidgetPath.h" 
#include "Framework/Application/SlateApplication.h" 
#include "Kismet/GameplayStatics.h" 

namespace EchoHUDTheme
{
	const FLinearColor ColorWhite = FLinearColor(0.95f, 0.95f, 0.95f, 1.0f);
	const FLinearColor ColorGrey = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor ColorDark = FLinearColor(0.05f, 0.05f, 0.05f, 0.85f);

	const FLinearColor ColorGlass = FLinearColor(0.8f, 0.9f, 1.0f, 0.3f);
	const FLinearColor ColorLineDim = FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);

	const FLinearColor ColorControlsText = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
	const FLinearColor ColorActive = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	const FLinearColor BarHealthFill = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
	const FLinearColor BarStaminaFill = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor BarBackground = FLinearColor(0.05f, 0.05f, 0.05f, 0.9f);
	const FLinearColor BarBorder = FLinearColor(0.5f, 0.5f, 0.5f, 0.5f);

	const FLinearColor ColorBlockSpecial = FLinearColor(0.7f, 0.7f, 0.95f, 1.0f);
}


void Acringe3_topdownHUD::BeginPlay()
{
	Super::BeginPlay();
	bIsPauseMenuVisible = false;

	if (GEngine && GEngine->GameViewport)
	{
		HUDWidget = SNew(SEchoInGameHUD).OwningHUD(this);
		GEngine->GameViewport->AddViewportWidgetContent(HUDWidget.ToSharedRef());
	}
}

void Acringe3_topdownHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HUDWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(HUDWidget.ToSharedRef());
	}
	if (PauseWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(PauseWidget.ToSharedRef());
	}
	Super::EndPlay(EndPlayReason);
}

void Acringe3_topdownHUD::TogglePauseMenu()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (bIsPauseMenuVisible)
	{
		if (PauseWidget.IsValid() && GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(PauseWidget.ToSharedRef());
			PauseWidget.Reset();
		}

		bIsPauseMenuVisible = false;
		UGameplayStatics::SetGamePaused(this, false);

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);

		PC->bShowMouseCursor = true;

		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
	else
	{
		if (GEngine && GEngine->GameViewport)
		{
			PauseWidget = SNew(SEchoPauseWidget).OwningHUD(this);
			GEngine->GameViewport->AddViewportWidgetContent(PauseWidget.ToSharedRef(), 100);

			FSlateApplication::Get().SetKeyboardFocus(PauseWidget);
		}

		bIsPauseMenuVisible = true;
		UGameplayStatics::SetGamePaused(this, true);

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void Acringe3_topdownHUD::SetAimingMode(bool bEnable, FVector2D Center, FVector2D MousePos, float AimAngleOffset)
{
	bIsAiming = bEnable;
	ScreenCenter = Center;
	CurrentMousePos = MousePos;
	CurrentRotationOffset = AimAngleOffset;
}

void Acringe3_topdownHUD::UpdateSelectedDirection(ECombatDirection NewDir)
{
	SelectedDirection = NewDir;
}

void SEchoInGameHUD::Construct(const FArguments& InArgs)
{
	OwnerHUD = InArgs._OwningHUD;
	SetCanTick(false);

	FSlateFontInfo MainFont = FCoreStyle::GetDefaultFontStyle("Regular", 14);
	FSlateFontInfo HeaderFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);

	ChildSlot
		[
			SNew(SOverlay)

				+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center).Padding(0, 0, 40, 0)
				[
					SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(EchoHUDTheme::ColorDark)
						.Padding(25)
						[
							SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 15)
								[
									SNew(STextBlock)
										.Text(FText::FromString("CONTROLS"))
										.Font(HeaderFont)
										.ColorAndOpacity(EchoHUDTheme::ColorWhite)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 15)
								[
									SNew(SBox).HeightOverride(2).WidthOverride(100)
										[
											SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(EchoHUDTheme::ColorGrey)
										]
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
								[
									SNew(STextBlock).Text(FText::FromString("LMB - Attack")).Font(MainFont).ColorAndOpacity(EchoHUDTheme::ColorControlsText)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
								[
									SNew(STextBlock).Text(FText::FromString("RMB - Aim")).Font(MainFont).ColorAndOpacity(EchoHUDTheme::ColorControlsText)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
								[
									SNew(STextBlock).Text(FText::FromString("MMB - Lock")).Font(MainFont).ColorAndOpacity(EchoHUDTheme::ColorControlsText)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
								[
									SNew(STextBlock).Text(FText::FromString("SPACE - Dash")).Font(MainFont).ColorAndOpacity(EchoHUDTheme::ColorControlsText)
								]
						]
				]
		];
}

void DrawProgressBar(FSlateWindowElementList& OutDrawElements, int32 LayerId, const FGeometry& AllottedGeometry, FVector2D Pos, FVector2D Size, float Percent, FLinearColor FillColor)
{
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(Size, FSlateLayoutTransform(Pos)), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, EchoHUDTheme::BarBackground);

	float Padding = 2.0f;
	float MaxFillWidth = Size.X - (Padding * 2);
	float FillHeight = Size.Y - (Padding * 2);
	float CurrentFillWidth = MaxFillWidth * FMath::Clamp(Percent, 0.0f, 1.0f);

	if (CurrentFillWidth > 0.0f)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(FVector2D(CurrentFillWidth, FillHeight), FSlateLayoutTransform(Pos + FVector2D(Padding, Padding))), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, FillColor);
	}

	TArray<FVector2D> Border;
	Border.Add(Pos);
	Border.Add(Pos + FVector2D(Size.X, 0));
	Border.Add(Pos + Size);
	Border.Add(Pos + FVector2D(0, Size.Y));
	Border.Add(Pos);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Border, ESlateDrawEffect::None, EchoHUDTheme::BarBorder, true, 1.0f);
}

void DrawDirectionIcon(FSlateWindowElementList& OutDrawElements, int32 LayerId, const FGeometry& AllottedGeometry, FVector2D Center, float AngleDeg, float Size, FLinearColor Color)
{
	float Rad = FMath::DegreesToRadians(AngleDeg);
	FVector2D IconCenter = Center;
	FVector2D Forward(FMath::Cos(Rad), FMath::Sin(Rad));
	FVector2D Right(-Forward.Y, Forward.X);

	FVector2D Tip = IconCenter - (Forward * (Size * 0.5f));
	FVector2D LeftTail = IconCenter + (Forward * (Size * 0.5f)) - (Right * (Size * 0.5f));
	FVector2D RightTail = IconCenter + (Forward * (Size * 0.5f)) + (Right * (Size * 0.5f));

	TArray<FVector2D> Points;
	Points.Add(LeftTail); Points.Add(Tip); Points.Add(RightTail);

	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, false, 2.0f);

	TArray<FVector2D> Shaft;
	Shaft.Add(IconCenter + (Forward * (Size * 0.6f)));
	Shaft.Add(Tip + (Forward * 2.0f));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Shaft, ESlateDrawEffect::None, Color.CopyWithNewOpacity(Color.A * 0.6f), false, 1.5f);
}

void SEchoInGameHUD::DrawArc(FSlateWindowElementList& OutDrawElements, int32 LayerId, const FGeometry& AllottedGeometry, FVector2D Center, float InnerRadius, float OuterRadius, float StartAngleDeg, float EndAngleDeg, FLinearColor Color, bool bIsOutline) const
{
	const int32 NumSegments = 16;
	TArray<FVector2D> Points;
	Points.Reserve((NumSegments + 1) * 2 + 1);

	float AngleStep = (EndAngleDeg - StartAngleDeg) / NumSegments;

	if (bIsOutline)
	{
		for (int32 i = 0; i <= NumSegments; i++)
		{
			float Rad = FMath::DegreesToRadians(StartAngleDeg + (i * AngleStep));
			Points.Add(Center + FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * OuterRadius);
		}
		for (int32 i = NumSegments; i >= 0; i--)
		{
			float Rad = FMath::DegreesToRadians(StartAngleDeg + (i * AngleStep));
			Points.Add(Center + FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * InnerRadius);
		}

		if (Points.Num() > 0)
		{
			FVector2D FirstPoint = Points[0];
			Points.Add(FirstPoint);
		}

		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, 1.0f);
	}
	else
	{
		float MidRadius = (InnerRadius + OuterRadius) * 0.5f;
		float Thickness = OuterRadius - InnerRadius;
		Points.Empty();
		for (int32 i = 0; i <= NumSegments; i++)
		{
			float Rad = FMath::DegreesToRadians(StartAngleDeg + (i * AngleStep));
			Points.Add(Center + FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * MidRadius);
		}
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, Thickness);
	}
}

int32 SEchoInGameHUD::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 RetLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (!OwnerHUD.IsValid()) return RetLayerId;

	APlayerController* PC = OwnerHUD->GetOwningPlayerController();
	if (!PC) return RetLayerId;

	float UIScale = AllottedGeometry.GetAccumulatedLayoutTransform().GetScale();
	if (UIScale <= 0.0f) UIScale = 1.0f;

	Acringe3_topdownCharacter* MyChar = Cast<Acringe3_topdownCharacter>(PC->GetPawn());

	if (MyChar)
	{
		float HPPct = MyChar->GetHealthPercent();
		float StaminaPct = MyChar->GetStaminaPercent();

		DrawProgressBar(OutDrawElements, LayerId, AllottedGeometry, FVector2D(50, 50), FVector2D(350, 16), HPPct, EchoHUDTheme::BarHealthFill);
		DrawProgressBar(OutDrawElements, LayerId, AllottedGeometry, FVector2D(50, 72), FVector2D(250, 8), StaminaPct, EchoHUDTheme::BarStaminaFill);
	}

	if (MyChar)
	{
		FVector2D CharScreenPosRaw;
		if (PC->ProjectWorldLocationToScreen(MyChar->GetActorLocation(), CharScreenPosRaw))
		{
			FVector2D CharScreenPos = CharScreenPosRaw / UIScale;

			FVector ForwardPoint = MyChar->GetActorLocation() + (MyChar->GetActorForwardVector() * 100.0f);
			FVector2D ForwardScreenPosRaw;
			PC->ProjectWorldLocationToScreen(ForwardPoint, ForwardScreenPosRaw);

			FVector2D Dir = ((ForwardScreenPosRaw / UIScale) - CharScreenPos).GetSafeNormal();
			float CharAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));

			float R_In = 30.0f;
			float Base_R_Out = 33.0f;

			struct FCompassSegment { int32 Offset; float Alpha; float Width; };
			FCompassSegment Segments[] = { { 0,  1.0f, 40.0f }, { -1, 0.5f, 30.0f }, { 1,  0.5f, 30.0f }, { -2, 0.2f, 20.0f }, { 2,  0.2f, 20.0f } };

			for (const auto& Seg : Segments)
			{
				float CenterAngle = CharAngleDeg + (Seg.Offset * 45.0f);
				float Current_R_Out = (Seg.Offset == 0) ? Base_R_Out + 2.0f : Base_R_Out;
				DrawArc(OutDrawElements, LayerId, AllottedGeometry, CharScreenPos, R_In, Current_R_Out, CenterAngle - Seg.Width * 0.5f, CenterAngle + Seg.Width * 0.5f, EchoHUDTheme::ColorWhite.CopyWithNewOpacity(Seg.Alpha), false);
			}

			if (MyChar->IsBlocking() && MyChar->GetCurrentBlockMode() == ECombatDirection::Thrust)
			{
				DrawArc(OutDrawElements, LayerId + 1, AllottedGeometry, CharScreenPos, 40.0f, 43.0f, CharAngleDeg - 30, CharAngleDeg + 30, EchoHUDTheme::ColorBlockSpecial, false);
			}
		}
	}

	if (MyChar && MyChar->IsTargetLocked())
	{
		AActor* Target = MyChar->GetLockedTarget();
		if (Target)
		{
			FVector2D TargetScreenPosRaw;
			if (PC->ProjectWorldLocationToScreen(Target->GetActorLocation(), TargetScreenPosRaw))
			{
				FVector2D TargetScreenPos = TargetScreenPosRaw / UIScale;
				float Time = MyChar->GetWorld()->GetTimeSeconds();
				float Size = 16.0f;
				float Angle = Time * 2.5f;

				FVector2D P1 = TargetScreenPos + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Size;
				FVector2D P2 = TargetScreenPos + FVector2D(FMath::Cos(Angle + UE_HALF_PI), FMath::Sin(Angle + UE_HALF_PI)) * Size;
				FVector2D P3 = TargetScreenPos + FVector2D(FMath::Cos(Angle + UE_PI), FMath::Sin(Angle + UE_PI)) * Size;
				FVector2D P4 = TargetScreenPos + FVector2D(FMath::Cos(Angle + 1.5f * UE_PI), FMath::Sin(Angle + 1.5f * UE_PI)) * Size;

				TArray<FVector2D> DiamondShape; DiamondShape.Add(P1); DiamondShape.Add(P2); DiamondShape.Add(P3); DiamondShape.Add(P4); DiamondShape.Add(P1);
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), DiamondShape, ESlateDrawEffect::None, EchoHUDTheme::ColorWhite, true, 2.0f);

				Acringe3_topdownCharacter* EnemyChar = Cast<Acringe3_topdownCharacter>(Target);
				if (EnemyChar) {
					DrawProgressBar(OutDrawElements, LayerId, AllottedGeometry, TargetScreenPos - FVector2D(35, 45), FVector2D(70, 6), EnemyChar->GetHealthPercent(), EchoHUDTheme::BarHealthFill);
				}
			}
		}
	}

	if (OwnerHUD->IsAiming())
	{
		FVector2D Center = OwnerHUD->GetScreenCenter() / UIScale;
		FVector2D MousePos = OwnerHUD->GetMousePos() / UIScale;
		float BaseAngleDeg = FMath::RadiansToDegrees(OwnerHUD->GetRotationOffset());
		int32 SelectedIdx = (int32)OwnerHUD->GetSelectedDir();

		float R_Inner = 150.0f;
		float R_Outer = 175.0f;
		float Gap = 3.0f;

		for (int32 i = 0; i < 8; i++)
		{
			float SectorCenterDeg = BaseAngleDeg + (i * 45.0f);
			float StartAngle = SectorCenterDeg - 22.5f + Gap;
			float EndAngle = StartAngle + 45.0f - (Gap * 2);
			bool bIsActive = (i == SelectedIdx);

			FLinearColor LineColor = EchoHUDTheme::ColorLineDim;
			FLinearColor FillColor = FLinearColor::Transparent;
			FLinearColor IconColor = EchoHUDTheme::ColorLineDim.CopyWithNewOpacity(0.3f);

			if (bIsActive)
			{
				bool bIsBlocking = (MyChar && MyChar->IsBlocking());

				if (bIsBlocking)
				{
					LineColor = EchoHUDTheme::ColorBlockSpecial;
					FillColor = EchoHUDTheme::ColorBlockSpecial.CopyWithNewOpacity(0.15f);
					IconColor = EchoHUDTheme::ColorWhite;
				}
				else
				{
					LineColor = EchoHUDTheme::ColorWhite;
					FillColor = EchoHUDTheme::ColorGlass;
					IconColor = EchoHUDTheme::ColorWhite;
				}

				DrawArc(OutDrawElements, LayerId, AllottedGeometry, Center, R_Inner, R_Outer + 5.0f, StartAngle, EndAngle, FillColor, false);
				DrawArc(OutDrawElements, LayerId + 1, AllottedGeometry, Center, R_Inner, R_Outer + 5.0f, StartAngle, EndAngle, LineColor, true);
			}
			else
			{
				DrawArc(OutDrawElements, LayerId, AllottedGeometry, Center, R_Inner, R_Outer, StartAngle, EndAngle, EchoHUDTheme::ColorLineDim, true);
			}

			float Rad = FMath::DegreesToRadians(SectorCenterDeg);
			FVector2D IconPos = Center + FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * ((R_Inner + R_Outer) * 0.5f);
			DrawDirectionIcon(OutDrawElements, LayerId + 2, AllottedGeometry, IconPos, SectorCenterDeg, 10.0f, IconColor);
		}

		TArray<FVector2D> LinePts; LinePts.Add(Center); LinePts.Add(MousePos);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), LinePts, ESlateDrawEffect::None, EchoHUDTheme::ColorWhite.CopyWithNewOpacity(0.3f), true, 0.5f);

		FString DirText = UEnum::GetValueAsString(OwnerHUD->GetSelectedDir()).Replace(TEXT("ECombatDirection::"), TEXT(""));
		FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular", 10);
		FVector2D TextOffset(0, R_Outer + 35);
		FSlateDrawElement::MakeText(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(FVector2D(200, 30), FSlateLayoutTransform(Center + TextOffset - FVector2D(25, 0))), DirText, Font, ESlateDrawEffect::None, EchoHUDTheme::ColorWhite);
	}

	return RetLayerId + 5;
}