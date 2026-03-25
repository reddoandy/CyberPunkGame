// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
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
	void LoginEOS(class APlayerController*PlayerController);

	FString CachedEosUserId;
	FDelegateHandle LoginHandle;
	FDelegateHandle ServerLoginCompleteHandle;
	FOnLoginCompleteDelegate LoginComplete;
	FTimerHandle MatchPollTimer;

	UFUNCTION(BlueprintCallable)
	void SendMatchRequest();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void SendMatchFoundResult(const FMatchResult& Result);

	UFUNCTION(BlueprintImplementableEvent)
	void ServerCompleteLogin();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayerCompleteLogin();

	UFUNCTION(BlueprintCallable)
	void GetLoggedUserId();

	UFUNCTION(BlueprintCallable)
	void CheckLoginEOS_Server();

	UFUNCTION(BlueprintCallable)
	void DebugEOSLoginState();

	UFUNCTION(BlueprintCallable)
	void SyncUniqueIdToPlayer(int32 LocalUserNum);

	//UFUNCTION(BlueprintCallable)
	//static UMyGameInstance*LoginEOSAuth(class APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	void SaveLogin(FString Id);

	//UFUNCTION(BlueprintCallable)
	//void MyFindSessions();

	UFUNCTION(BlueprintImplementableEvent)
	void ReadyToJoinMatchEvent(const int32 &MatchPort);

	UFUNCTION(BlueprintCallable)
	void MyJoinMatch(class APlayerController *PlayerController ,int Port);

	UFUNCTION(Blueprintcallable)
	void PullTeamInfo(FString Id, FString& Team, int32& Index);


private:
	void OnLoginComplete(
		int32 LocalUserNum,
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const FString& Error
	);

	void OnCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorVal);

	IOnlineIdentityPtr Identity;
	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	FOnLoginCompleteDelegate Delegate;

	FDelegateHandle DelegateHandle;

	void OnServerLoginComplete(
		int32 LocalUserNum,
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const FString& Error);

	void StartPollingMatchStatus();

	void StopPollingMatchStatus();

	void PollingMatchStatus();

};


