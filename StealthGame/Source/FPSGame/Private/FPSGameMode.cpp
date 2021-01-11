// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "FPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "FPSGameState.h"


AFPSGameMode::AFPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_Player"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSHUD::StaticClass();

	//spawn the current game state (important because of CompleteMission() at the end of this file watch the code!)
	GameStateClass = AFPSGameState::StaticClass();
}

void AFPSGameMode::CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess)
{
	if(InstigatorPawn)
	{
		// we don't need the player controller to be enabled (you can check the implementation of the function
		// disables only game play input stuff
		//InstigatorPawn->DisableInput(nullptr); (putting OnMissionCompelete under instread for networking !)

		if(SpectatingViewpointClass)
		{
			TArray<AActor*> ReturnedActors;
			UGameplayStatics::GetAllActorsOfClass(this, SpectatingViewpointClass, ReturnedActors);

			AActor* NewViewTarget = nullptr;
			// change the view target if any valid actor found
			if (ReturnedActors.Num() > 0)
			{
				NewViewTarget = ReturnedActors[0];
				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
				{
					APlayerController* PC = It->Get();
					if (PC)
					{
						PC->SetViewTargetWithBlend(NewViewTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
					}
				}
				
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SpectatingViewPointClass is nullptr. Please update GameMode class with valid subclass. Cannot change spectating view actor."));
		}

	}

	AFPSGameState* GS = GetGameState<AFPSGameState>();
	if (GS)
	{
		GS->MulticastOnMissionComplete(InstigatorPawn, bMissionSuccess);
	}

	//blueprint funciton
	OnMissionCompleted(InstigatorPawn, bMissionSuccess);

}

