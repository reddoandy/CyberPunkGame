// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "HTTP.h"

void UMyGameInstance::Init() 
{
	Super::Init();

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EOS"));
	if (!Subsystem) 
	{
		UE_LOG(LogTemp, Error, TEXT("EOS Subsystem not found"));
		return;
	}

	Identity = Subsystem->GetIdentityInterface();
}

void UMyGameInstance::LoginEOS()
{
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Identity invalid"));
		return;
	}

	if (LoginCompleteHandle.IsValid())
	{
		Identity->OnLoginCompleteDelegates->Remove(LoginCompleteHandle);
		LoginCompleteHandle.Reset();
	}

	LoginCompleteHandle =
		Identity->OnLoginCompleteDelegates->AddUObject(
			this,
			&UMyGameInstance::OnLoginComplete
		);

	FOnlineAccountCredentials Credentials;
	Credentials.Type = TEXT("accountportal");
	Credentials.Id = TEXT("");
	Credentials.Token = TEXT("");

	Identity->Login(0, Credentials);
}

void UMyGameInstance::OnLoginComplete(
	int32 LocalUserNum,
	bool bWasSuccessful,
	const FUniqueNetId& UserId,
	const FString& Error
)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("EOS Login failed: %s"), *Error);
		return;
	}

	CachedEosUserId = UserId.ToString();

	UE_LOG(LogTemp, Log, TEXT("EOS Login success: %s"), *CachedEosUserId);
}

void UMyGameInstance::SendMatchRequest()
{
	if (CachedEosUserId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("EOS UserId empty"));
		return;
	}

	TSharedRef<IHttpRequest> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL("http://52.62.244.88:5140/api/match/join");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	FString Body = FString::Printf(
		TEXT("{\"eosId\":\"%s\"}"),
		*CachedEosUserId
	);

	Request->SetContentAsString(Body);

	Request->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (!bSuccess || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("HTTP failed or invalid response"));
				return;
			}

			UE_LOG(LogTemp, Log, TEXT("Backend response: %s"),
				*Resp->GetContentAsString());
		}
	);

	Request->ProcessRequest();
}





