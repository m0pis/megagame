#include "cringe3_MenuGameMode.h"
#include "cringe3_MenuHUD.h"
#include "GameFramework/PlayerController.h"

ACringe3_MenuGameMode::ACringe3_MenuGameMode()
{
	HUDClass = ACringe3_MenuHUD::StaticClass();

	DefaultPawnClass = nullptr;

	PlayerControllerClass = APlayerController::StaticClass();
}