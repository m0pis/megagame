#include "cringe3_ParticleWidget.h"
#include "Rendering/DrawElements.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

const int32 MAX_PARTICLES = 150;

void SParticleBackground::Construct(const FArguments& InArgs)
{
	LastUpdateTime = FPlatformTime::Seconds();
	AccumulatedRealTime = 0.0f;

	FString PathToMat = TEXT("/Script/Engine.Material'/Game/UI/M_ProceduralSmoke.M_ProceduralSmoke'");

	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(NULL, *PathToMat);

	if (BaseMat)
	{
		SmokeMID = UMaterialInstanceDynamic::Create(BaseMat, NULL);

		SmokeShaderBrush.SetResourceObject(SmokeMID);
		SmokeShaderBrush.ImageSize = FVector2D(1920, 1080);
		SmokeShaderBrush.DrawAs = ESlateBrushDrawType::Image;
		SmokeShaderBrush.TintColor = FSlateColor(FLinearColor::White);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CRITICAL ERROR: Could not find Material at path: %s"), *PathToMat);
		SmokeShaderBrush.SetResourceObject(FCoreStyle::Get().GetBrush("WhiteBrush")->GetResourceObject());
		SmokeShaderBrush.TintColor = FSlateColor(FLinearColor::Black);
	}

	for (int i = 0; i < MAX_PARTICLES; i++) SpawnParticle(true);
}

void SParticleBackground::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (SmokeMID)
	{
		Collector.AddReferencedObject(SmokeMID);
	}
}

void SParticleBackground::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (SmokeMID)
	{
		AccumulatedRealTime += InDeltaTime;

		SmokeMID->SetScalarParameterValue(TEXT("UserTime"), AccumulatedRealTime);
	}
	// --------------------------------------

	if (Particles.Num() < MAX_PARTICLES) SpawnParticle(false);

	for (int32 i = Particles.Num() - 1; i >= 0; i--)
	{
		FSimpleParticle& P = Particles[i];
		P.Position += P.Velocity * InDeltaTime;
		P.Position.X += FMath::Sin(InCurrentTime * 5.0f + P.Position.Y * 0.1f) * 20.0f * InDeltaTime;
		P.LifeTime -= InDeltaTime;
		P.Opacity = FMath::Min(P.LifeTime, 1.0f);
		if (P.LifeTime <= 0.0f || P.Position.Y < -50.0f) Particles.RemoveAtSwap(i);
	}
}

void SParticleBackground::SpawnParticle(bool bRandomStart)
{
	FSimpleParticle P;
	P.Size = FMath::RandRange(1.5f, 3.0f);
	float StartY = bRandomStart ? FMath::RandRange(0.0f, 1080.0f) : 1100.0f;
	P.Position = FVector2D(FMath::RandRange(0.0f, 1920.0f), StartY);
	P.Velocity = FVector2D(FMath::RandRange(-10.0f, 10.0f), FMath::RandRange(-100.0f, -40.0f));
	P.MaxLifeTime = FMath::RandRange(3.0f, 8.0f);
	P.LifeTime = P.MaxLifeTime;
	Particles.Add(P);
}

int32 SParticleBackground::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		&SmokeShaderBrush,
		ESlateDrawEffect::None,
		FLinearColor::White
	);

	const FSlateBrush* SolidBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	for (const FSimpleParticle& P : Particles)
	{
		FLinearColor SparkColor = FLinearColor(0.8f, 0.8f, 0.8f, P.Opacity);
		FSlateLayoutTransform LayoutTransform(P.Position);
		FPaintGeometry PaintGeo = AllottedGeometry.ToPaintGeometry(FVector2D(P.Size, P.Size), LayoutTransform);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 1,
			PaintGeo,
			SolidBrush,
			ESlateDrawEffect::None,
			SparkColor
		);
	}

	return LayerId + 1;
}