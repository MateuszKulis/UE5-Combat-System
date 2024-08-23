// Copyright Epic Games, Inc. All Rights Reserved.

#include "SoulsLikeCombatGameMode.h"
#include "SoulsLikeCombatCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASoulsLikeCombatGameMode::ASoulsLikeCombatGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
