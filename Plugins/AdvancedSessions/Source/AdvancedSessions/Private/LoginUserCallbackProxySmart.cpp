// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "LoginUserCallbackProxySmart.h"

#include "Online.h"

//#include "../../../../../Source/CyberPunkGame/MyGameInstance.h"
//#include "MyGameInstance.h"

//////////////////////////////////////////////////////////////////////////
// ULoginUserCallbackProxy

ULoginUserCallbackProxySmart::ULoginUserCallbackProxySmart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Delegate(FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnCompleted))
{
}

ULoginUserCallbackProxySmart* ULoginUserCallbackProxySmart::LoginUserSmart(UObject* WorldContextObject, class APlayerController* PlayerController, FString UserID, FString UserToken, FString AuthType)
{
	ULoginUserCallbackProxySmart* Proxy = NewObject<ULoginUserCallbackProxySmart>();
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->UserID = UserID;
	Proxy->UserToken = UserToken;
	Proxy->AuthType = "accountportal";
	Proxy->WorldContextObject = WorldContextObject;
	return Proxy;
}

void ULoginUserCallbackProxySmart::Activate()
{

	if (!PlayerControllerWeakPtr.IsValid())
	{
		OnFailure.Broadcast();
		return;
	}

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);

	if (!Player)
	{
		OnFailure.Broadcast();
		return;
	}

	FOnlineSubsystemBPCallHelperAdvanced Helper(TEXT("LoginUser"), GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull));

	if (!Helper.OnlineSub)
	{
		OnFailure.Broadcast();
		return;
	}

	auto Identity = Helper.OnlineSub->GetIdentityInterface();
	if (Identity.IsValid())
	{
		int32 ControllerId = Player->GetControllerId();

		TSharedPtr<const FUniqueNetId> ExistingId = Identity->GetUniquePlayerId(ControllerId);
		ELoginStatus::Type Status = Identity->GetLoginStatus(ControllerId);

		if (Status == ELoginStatus::LoggedIn && ExistingId.IsValid())
		{
			FUniqueNetIdRepl UniqueID(ExistingId);

			// 重新綁定 LocalPlayer UniqueId
			Player->SetCachedUniqueNetId(UniqueID);

			// 重新綁定 PlayerState UniqueId
			if (APlayerState* State = PlayerControllerWeakPtr->PlayerState)
			{
				State->SetUniqueId(UniqueID);
			}

			UE_LOG(LogTemp, Warning, TEXT("[AdvancedSessions FIX] Already logged in, rebind UniqueNetId OK"));

			OnSuccess.Broadcast();
			return;
		}

		// Fallback to default AuthType if nothing is specified
		if (AuthType.IsEmpty())
		{
			AuthType = Identity->GetAuthType();
		}
		DelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(Player->GetControllerId(), Delegate);
		FOnlineAccountCredentials AccountCreds(AuthType, UserID, UserToken);
		Identity->Login(Player->GetControllerId(), AccountCreds);
		return;
	}

	// Fail immediately
	OnFailure.Broadcast();
}

void ULoginUserCallbackProxySmart::OnCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorVal)
{
	if (PlayerControllerWeakPtr.IsValid())
	{
		ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);

		FUniqueNetIdRepl UniqueID(UserId.AsShared());

		if (Player)
		{
			FOnlineSubsystemBPCallHelperAdvanced Helper(TEXT("GetUserPrivilege"), GEngine->GetWorldFromContextObject(WorldContextObject.Get(), EGetWorldErrorMode::LogAndReturnNull));

			if (!Helper.OnlineSub)
			{
				OnFailure.Broadcast();
				return;
			}

			auto Identity = Helper.OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				Identity->ClearOnLoginCompleteDelegate_Handle(Player->GetControllerId(), DelegateHandle);
			}
			Player->SetCachedUniqueNetId(UniqueID);
		}

		if (APlayerState* State = PlayerControllerWeakPtr->PlayerState)
		{
			// Update UniqueId. See also ShowLoginUICallbackProxy.cpp
			State->SetUniqueId(UniqueID);
		}



	}

	if (bWasSuccessful)
	{
		OnSuccess.Broadcast();
	}
	else
	{
		OnFailure.Broadcast();
	}
}
