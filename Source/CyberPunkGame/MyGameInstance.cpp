// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "HTTP.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

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
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Identity interface invalid"));
		return;
	}

	// 嘗試取得 LocalUserNum = 0 的 UserId
	TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(0);
	if (UserId.IsValid())
	{
		CachedEosUserId = UserId->ToString();
		UE_LOG(LogTemp, Log, TEXT("Found EOS UserId: %s"), *CachedEosUserId);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No EOS user found, need login"));
		// 這裡可以觸發 LoginEOS()
	}
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

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL("http://54.206.115.20:5140/api/match/request");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("EosId"), CachedEosUserId);
	Json->SetStringField(TEXT("region"), TEXT("ap-southeast-2a"));

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer =
		TJsonWriterFactory<>::Create(&RequestBody);

	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestBody);

	Request->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (!bSuccess || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("HTTP failed or invalid response"));
				return;
			}

			FString ResponseStr = Resp->GetContentAsString();
			UE_LOG(LogTemp, Log, TEXT("Backend response: %s"), *ResponseStr);

			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(ResponseStr);

			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) 
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
				return;
			}

			FMatchResult MatchResult;

			MatchResult.IsMatched = JsonObject->GetBoolField(TEXT("matched"));

			JsonObject->TryGetStringField(TEXT("matchId"),MatchResult.MatchId);

			const TArray<TSharedPtr<FJsonValue>>* PlayersArray;
			if (JsonObject->TryGetArrayField(TEXT("players"), PlayersArray)) 
			{
				for (const TSharedPtr<FJsonValue>& Val : *PlayersArray) 
				{
					MatchResult.Players.Add(Val->AsString());
				}
			}
			SendMatchFoundResult(MatchResult);
		}
	);

	Request->ProcessRequest();
}





