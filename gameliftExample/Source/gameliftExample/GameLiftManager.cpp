// Fill out your copyright notice in the Description page of Project Settings.

#include "gameliftExample.h"
#include "GameLiftManager.h"



// Sets default values
AGameLiftManager::AGameLiftManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameLiftManager::BeginPlay()
{
	Super::BeginPlay();

	SetUpAwsClient();
}

// Called every frame
void AGameLiftManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AGameLiftManager::SetUpAwsClient()
{
	Aws::Client::ClientConfiguration config;

	config.scheme = Aws::Http::Scheme::HTTPS;
	config.connectTimeoutMs = 30000;
	config.requestTimeoutMs = 30000;
	config.region = Aws::Region::AP_NORTHEAST_1;

	GLClient = Aws::MakeShared<Aws::GameLift::GameLiftClient>("GameLiftTest", config);
}

bool AGameLiftManager::SearchGameSessions()
{
	Aws::GameLift::Model::SearchGameSessionsRequest req;

	req.SetAliasId(TCHAR_TO_UTF8(*AliasId));
	req.SetFilterExpression("hasAvailablePlayerSessions=true");

	auto outcome = GLClient->SearchGameSessions(req);

	if (outcome.IsSuccess())
	{
		auto& games = outcome.GetResult().GetGameSessions();
		auto& gs = games[0];

		Port = gs.GetPort();
		ServerIpAddress = UTF8_TO_TCHAR(gs.GetIpAddress().c_str());
		GameSessionId = UTF8_TO_TCHAR(gs.GetGameSessionId().c_str());
		return true;
	}

	return false;
}

bool AGameLiftManager::CreatePlayerSession()
{
	Aws::GameLift::Model::CreatePlayerSessionRequest req;
	req.SetGameSessionId(TCHAR_TO_UTF8(*GameSessionId));
	req.SetPlayerId("PlayerUe4");

	auto outcome = GLClient->CreatePlayerSession(req);

	if (outcome.IsSuccess())
	{
		auto& psess = outcome.GetResult().GetPlayerSession();
		PlayerSessionId = UTF8_TO_TCHAR(psess.GetPlayerSessionId().c_str());

		return true;
	}

	return false;
}

bool AGameLiftManager::ConnectPlayerSession()
{
	return false;
}

void AGameLiftManager::DoTestNow()
{
	if (!SearchGameSessions())
		return;
	
	if (!CreatePlayerSession())
		return;

	if (!ConnectPlayerSession())
		return;
}
