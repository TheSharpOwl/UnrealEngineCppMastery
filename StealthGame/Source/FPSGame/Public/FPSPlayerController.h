// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// we use controller because for each player we have 1 controller (his)
	// while for each player we always have N pawns (N = the total number of pawns)
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
	void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);
};
