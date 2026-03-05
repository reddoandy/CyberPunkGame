// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Online.h"
#include "MyPlayerControllerCpp.h"
#include <Kismet/GameplayStatics.h>
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

void UMyGameInstance::Init() 
{
	Super::Init();

	if (IsRunningDedicatedServer()) 
	{
		FTimerHandle TempHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TempHandle,
			this,
			&UMyGameInstance::CheckLoginEOS_Server,
			1.0f,
			false
		);
		
	}
	//else
	//{
		//GetLoggedUserId();

		//if (UWorld* World = GetWorld()) 
		//{
			//World->GetTimerManager().SetTimer(
			//	EOSLoginTimerHandle,
				//this,
				//&UMyGameInstance::LoginEOS,
				//2.0f,
				//false
			//);
		//}
		
	//}
	
}

void UMyGameInstance::DebugEOSLoginState()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EOS"));
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EOS subsystem not found"));
		return;
	}

	Identity= Subsystem->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EOS Identity invalid"));
		return;
	}

	ELoginStatus::Type Status = Identity->GetLoginStatus(0);

	UE_LOG(LogTemp, Warning, TEXT("LoginStatus(0) = %d"), (int32)Status);

	TSharedPtr<const FUniqueNetId> NetId = Identity->GetUniquePlayerId(0);

	if (NetId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UniqueNetId(0) = %s"), *NetId->ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UniqueNetId(0) is NULL"));
	}
}

void UMyGameInstance::CheckLoginEOS_Server()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EOS"));
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EOS subsystem not found"));
		return;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("EOS is ready"));
		ServerCompleteLogin();
	}

}

void UMyGameInstance::OnServerLoginComplete(
	int32 LocalUserNum,
	bool bWasSuccessful,
	const FUniqueNetId& UserId,
	const FString& Error
)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("Server EOS login failed: %s"), *Error);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Dedicated Server EOS login success: %s"), *UserId.ToString());

	//  這裡才可以 Create Advanced Session
	ServerCompleteLogin();
}

void UMyGameInstance::GetLoggedUserId() 
{
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
		PlayerCompleteLogin();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No EOS user found, need login"));
		// 這裡可以觸發 LoginEOS()
		//LoginEOS();
	}
}

void UMyGameInstance::SaveLogin(FString Id) 
{
	CachedEosUserId = Id;
}

void UMyGameInstance::LoginEOS(class APlayerController*PlayerController)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EOS"));
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EOS subsystem not found"));
		return;
	}

	Identity = Subsystem->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Identity invalid"));
		return;
	}
	PlayerControllerWeakPtr = PlayerController;

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalPlayer not ready, retry login..."));
		//GetTimerManager().SetTimerForNextTick(this, &UMyGameInstance::LoginEOS(PlayerController));
		return;
	}

	int32 UserNum = LocalPlayer->GetControllerId();
	UE_LOG(LogTemp, Warning, TEXT("LoginEOS: ControllerId = %d"), UserNum);

	// 如果已經登入就直接同步
	//if (Identity->GetLoginStatus(UserNum) == ELoginStatus::LoggedIn)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Already logged in, syncing UniqueNetId"));
	//	TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(UserNum);
	//	CachedEosUserId = UserId->ToString();
	//	UE_LOG(LogTemp, Log, TEXT("Found EOS UserId: %s"), *CachedEosUserId);
		
	//}

	UE_LOG(LogTemp, Warning, TEXT("Calling EOS Login..."));
	if (LoginHandle.IsValid())
	{
		Identity->ClearOnLoginCompleteDelegate_Handle(UserNum, LoginHandle);
		LoginHandle.Reset();
	}

	
	// Delegate
	//LoginComplete = FOnLoginCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnLoginComplete);
	LoginHandle = Identity->AddOnLoginCompleteDelegate_Handle(LocalPlayer->GetControllerId(), FOnLoginCompleteDelegate::CreateUObject(this, &UMyGameInstance::OnLoginComplete));
	

	FOnlineAccountCredentials Credentials;
	Credentials.Type = TEXT("accountportal");
	Credentials.Id = TEXT("");
	Credentials.Token = TEXT("");
	UE_LOG(LogTemp, Warning, TEXT("Calling Identity->Login()..."));
	Identity->Login(UserNum, Credentials);

}

void UMyGameInstance::OnLoginComplete(
	int32 LocalUserNum,
	bool bWasSuccessful,
	const FUniqueNetId& UserId,
	const FString& Error
)
{
	if (Identity.IsValid() && LoginHandle.IsValid())
	{
		Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginHandle);
		LoginHandle.Reset();
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("EOS Login Failed: %s"), *Error);
		return;
	}

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);

	

	CachedEosUserId = UserId.ToString();
	//FUniqueNetIdRepl NetId(UserId.AsShared());
	TSharedPtr<const FUniqueNetId> NetId = UserId.AsShared();
	//FUniqueNetIdRepl NetId(CachedEosUserId);

	LocalPlayer->SetCachedUniqueNetId(NetId);

	UE_LOG(LogTemp, Log, TEXT("EOS Login success: %s"), *CachedEosUserId);

	if (APlayerState* PS = PlayerControllerWeakPtr->PlayerState) 
	{
		PS->SetUniqueId(UserId.AsShared());
		//UE_LOG(LogTemp, Warning, TEXT("PlayerState UniqueId set: %s"), *NetId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is NULL (may not be created yet)"));
	}


	PlayerCompleteLogin();
	//SyncUniqueIdToPlayer(LocalUserNum);
}

void UMyGameInstance::SyncUniqueIdToPlayer(int32 LocalUserNum)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("SyncUniqueIdToPlayer: PC null"));
		return;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		UE_LOG(LogTemp, Error, TEXT("SyncUniqueIdToPlayer: LocalPlayer null"));
		return;
	}

	TSharedPtr<const FUniqueNetId> NetId = Identity->GetUniquePlayerId(LocalUserNum);
	if (!NetId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SyncUniqueIdToPlayer: UniqueNetId invalid"));
		return;
	}

	FUniqueNetIdRepl ReplId(NetId);

	// LocalPlayer cache
	LP->SetCachedUniqueNetId(ReplId);

	// PlayerState UniqueId
	if (APlayerState*PS=PC->PlayerState.Get())
	{
		PS->SetUniqueId(ReplId);
	}
	PlayerCompleteLogin();
	UE_LOG(LogTemp, Warning, TEXT("UniqueNetId synced: %s"), *NetId->ToString());
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
	Request->SetURL(TEXT("http://43.213.182.84:5140/api/match/queue"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));


	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("EosId"), CachedEosUserId);
	Json->SetNumberField(TEXT("MMR"), 0);//這邊之後改成MMR

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
				UE_LOG(LogTemp, Warning, TEXT("Match request success"));
				UE_LOG(LogTemp, Log, TEXT("PlayerId:%s"),*CachedEosUserId);
				StartPollingMatchStatus();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Backend returned error: %d"), Code);
			}
		}
	);

	
	Request->ProcessRequest();
}

void UMyGameInstance::StartPollingMatchStatus()
{
	GetWorld()->GetTimerManager().SetTimer(
		MatchPollTimer,
		this,
		&UMyGameInstance::PollingMatchStatus,
		3.0f,
		true
	);
}

void UMyGameInstance::StopPollingMatchStatus()
{
	GetWorld()->GetTimerManager().ClearTimer(MatchPollTimer);
}

void UMyGameInstance::PollingMatchStatus()
{
	FString Url = FString::Printf(
		TEXT("http://43.213.182.84:5140/api/match/check"));

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL(TEXT("http://43.213.182.84:5140/api/match/check"));
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("PlayerId"), CachedEosUserId);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer =
		TJsonWriterFactory<>::Create(&RequestBody);

	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	Request->SetContentAsString(RequestBody);

	Request->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (!bSuccess || !Resp.IsValid())
				return;

			FString ResponseStr = Resp->GetContentAsString();

			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(ResponseStr);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) &&
				JsonObject.IsValid())
			{
				bool Ready =false;
				JsonObject->TryGetBoolField(TEXT("ServerReady"), Ready);

				if (Ready == true) 
				{
					StopPollingMatchStatus();
					int matchport;
					JsonObject->TryGetNumberField(TEXT("MatchPort"), matchport);
					ReadyToJoinMatchEvent(matchport);
				}
			}
		}
	);

	Request->ProcessRequest();
}

void UMyGameInstance::MyJoinMatch(class APlayerController *PC, int32 port) 
{
	FString Address = FString::Printf(TEXT("43.213.182.84:%d"), port);
	PC->ClientTravel(Address, TRAVEL_Absolute);
	UE_LOG(LogTemp, Log, TEXT("Join %s"), *Address);
}



