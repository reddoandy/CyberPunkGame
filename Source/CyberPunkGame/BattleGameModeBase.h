// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BattleGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class CYBERPUNKGAME_API ABattleGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly)
	FString MatchId;

	UFUNCTION(BlueprintCallable)
	void GetMatchId();

	UFUNCTION(BlueprintCallable)
	void SessionReady();
	
	
};
