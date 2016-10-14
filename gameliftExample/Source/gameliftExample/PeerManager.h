// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PeerManager.generated.h"

class ATcpClient;

UCLASS()
class GAMELIFTEXAMPLE_API APeerManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APeerManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	template<typename ...Args>
	void PerformFunction(void (APeerManager::*func)(Args...), Args... args)
	{
		PerformQueue.Enqueue([this, func, args...]()
		{
			(this->*func)(args...);
		});
	}

	void UpdateMe(float x, float y);
	void UpdatePeer(int id, float x, float y);
	void ChatDraw(FString from, FString chat);

private:
	UPROPERTY(EditAnywhere)
	UClass* MeClass;

	UPROPERTY(EditAnywhere)
	UClass* PeerClass;

	TQueue<std::function<void()>> PerformQueue;
	AActor* Me;
	TMap<int, AActor*> PeerMap;
	ATcpClient* Tcp;
};
