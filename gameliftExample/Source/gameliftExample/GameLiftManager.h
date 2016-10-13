// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GameLiftManager.generated.h"

UCLASS()
class GAMELIFTEXAMPLE_API AGameLiftManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameLiftManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void SetUpAwsClient();

	bool SearchGameSessions();
	bool CreatePlayerSession();
	bool ConnectPlayerSession();

	std::shared_ptr<Aws::GameLift::GameLiftClient> GetAwsClient()
	{
		return GLClient;
	}

	void DoTestNow();

private:
	std::shared_ptr<Aws::GameLift::GameLiftClient> GLClient;
	
	int Port;

	UPROPERTY(EditAnywhere)
	FString AliasId;
	FString ServerIpAddress;
	FString GameSessionId;
	FString PlayerSessionId;
};
