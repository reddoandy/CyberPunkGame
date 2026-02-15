// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerControllerCpp.h"

#include "GameFramework/PlayerState.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

void AMyPlayerControllerCpp::CopyUniqueIdToPlayerState() 
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EOS"));
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EOS subsystem not found"));
		return;
	}

	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOS Identity invalid"));
		return;
	}

	TSharedPtr<const FUniqueNetId> NetId = Identity->GetUniquePlayerId(0);
	if (!NetId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GetUniquePlayerId(0) failed"));
		return;
	}

	if (!PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerState is null"));
		return;
	}

	FUniqueNetIdRepl ReplId(NetId);
	PlayerState->SetUniqueId(ReplId);

	UE_LOG(LogTemp, Warning, TEXT("Synced UniqueNetId to PlayerState: %s"), *NetId->ToString());
}



