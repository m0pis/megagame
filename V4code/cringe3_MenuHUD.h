#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "cringe3_MenuHUD.generated.h"

UCLASS()
class ACringe3_MenuHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	void PlayGame();
	void QuitGame();

private:
	TSharedPtr<class SMainMenuWidget> MenuWidget;
};