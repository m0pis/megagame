#include "cringe3_MenuHUD.h"
#include "cringe3_MenuWidget.h"
#include "cringe3_LoadingWidget.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MoviePlayer.h" 

void ACringe3_MenuHUD::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine && GEngine->GameViewport)
	{
		MenuWidget = SNew(SMainMenuWidget).OwningHUD(this);
		GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef());

		if (APlayerController* PC = GetOwningPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}

		FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
	}
}

void ACringe3_MenuHUD::PlayGame()
{
	if (GEngine && GEngine->GameViewport && MenuWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
		MenuWidget.Reset();
	}

	if (GetMoviePlayer())
	{
		FLoadingScreenAttributes LoadingScreen;

		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.bMoviesAreSkippable = false;
		LoadingScreen.bWaitForManualStop = false;

		LoadingScreen.MinimumLoadingScreenDisplayTime = 2.0f;

		LoadingScreen.WidgetLoadingScreen = SNew(SLoadingWidget);

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}

	UGameplayStatics::OpenLevel(this, FName("TopDownMap"));
}

void ACringe3_MenuHUD::QuitGame()
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
	}
}