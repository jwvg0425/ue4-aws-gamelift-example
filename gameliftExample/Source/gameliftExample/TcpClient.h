// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AllowWindowsPlatformTypes.h"
#ifndef _WIN32
#include <netdb.h>
#include <unistd.h>
#else
#include <winsock2.h>
#endif
#include "HideWindowsPlatformTypes.h"
#include "Buffer.h"

#include "TcpClient.generated.h"

#define BUF_SIZE 32768

class APeerManager;

UCLASS()
class GAMELIFTEXAMPLE_API ATcpClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATcpClient();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	bool Connect(const FString& serverAddr, int port);

	void LoginRequest(const FString& playerSessionId);
	void ChatRequest(const FString& chat);
	void MoveRequest(float x, float y);

private:
	bool Initialize();
	bool Send(const char* data, int length);

	void NetworkThread();
	void ProcessPacket();

private:
	SOCKET Socket;
	Buffer RecvBuffer;
	int LoginId;

	APeerManager* PeerManager;
	
};
