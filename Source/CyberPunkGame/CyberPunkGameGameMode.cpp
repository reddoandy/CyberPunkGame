// Copyright Epic Games, Inc. All Rights Reserved.

#include "CyberPunkGameGameMode.h"
#include "CyberPunkGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACyberPunkGameGameMode::ACyberPunkGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
