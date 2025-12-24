// Copyright Epic Games, Inc. All Rights Reserved.

#include "cringe3_topdownGameMode.h"
#include "cringe3_topdownPlayerController.h"
#include "cringe3_topdownCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "cringe3_topdownHUD.h"

Acringe3_topdownGameMode::Acringe3_topdownGameMode()
{
	PlayerControllerClass = Acringe3_topdownPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	HUDClass = Acringe3_topdownHUD::StaticClass();
}