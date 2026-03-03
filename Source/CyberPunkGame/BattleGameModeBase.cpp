// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameModeBase.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"


void ABattleGameModeBase::GetMatchId()
{
	FParse::Value(FCommandLine::Get(), TEXT("MatchId="), MatchId);
}

void ABattleGameModeBase::SessionReady()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://43.213.182.84:5140/api/match/Match-ready"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));


	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("matchId"), MatchId);

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

			int32 Code = Resp->GetResponseCode();

			if (Code == 200)
			{
				UE_LOG(LogTemp, Warning, TEXT("Match Create success"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Backend returned error: %d"), Code);
			}
		}
	);


	Request->ProcessRequest();
}

