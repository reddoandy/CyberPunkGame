// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FMatchResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool IsMatched;

	UPROPERTY(BlueprintReadOnly)
	FString MatchId;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Players;
};


UCLASS()
class CYBERPUNKGAME_API UMyGameInstance : public UGameInstance
{
  GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void LoginEOS();

	FString CachedEosUserId;
	FDelegateHandle LoginCompleteHandle;

	UFUNCTION(BlueprintCallable)
	void SendMatchRequest();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void SendMatchFoundResult(const FMatchResult& Result);

private:
	void OnLoginComplete(
		int32 LocalUserNum,
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const FString& Error
	);

	IOnlineIdentityPtr Identity;

};


