// Fill out your copyright notice in the Description page of Project Settings.

#include "gameliftExample.h"
#include "PeerManager.h"
#include "TcpClient.h"
#include "GameLiftManager.h"


// Sets default values
APeerManager::APeerManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APeerManager::BeginPlay()
{
	Super::BeginPlay();
	
	Me = GetWorld()->SpawnActor(MeClass);

	TActorIterator<ATcpClient> iter(GetWorld());

	Tcp = *iter;
}

// Called every frame
void APeerManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	std::function<void()> item;

	if (PerformQueue.Dequeue(item))
		item();

	auto controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (controller->WasInputKeyJustReleased(EKeys::Left))
		Tcp->MoveRequest(Me->GetActorLocation().X - 10, Me->GetActorLocation().Y);
	
	if (controller->WasInputKeyJustReleased(EKeys::Up))
		Tcp->MoveRequest(Me->GetActorLocation().X, Me->GetActorLocation().Y - 10);
	
	if (controller->WasInputKeyJustReleased(EKeys::Right))
		Tcp->MoveRequest(Me->GetActorLocation().X + 10, Me->GetActorLocation().Y);

	if (controller->WasInputKeyJustReleased(EKeys::Down))
		Tcp->MoveRequest(Me->GetActorLocation().X, Me->GetActorLocation().Y + 10);

	if(controller->WasInputKeyJustReleased(EKeys::C))
		Tcp->ChatRequest(" - HELLO~~ THIS IS TEST...");

	if (controller->WasInputKeyJustReleased(EKeys::L))
	{
		TActorIterator<AGameLiftManager> iter(GetWorld());

		iter->DoTestNow();
	}
}

void APeerManager::UpdateMe(float x, float y)
{
	Me->SetActorLocation({ x,y,50.0f });
}

void APeerManager::UpdatePeer(int id, float x, float y)
{
	if (!PeerMap.Contains(id))
	{
		auto newPeer = GetWorld()->SpawnActor(PeerClass);
		newPeer->SetActorLocation({ x,y,50.0f });

		PeerMap.Add(id, newPeer);
	}
	else
	{
		PeerMap[id]->SetActorLocation({ x,y,50.0f });
	}
}

void APeerManager::ChatDraw(FString from, FString chat)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "CHAT FROM ID[" + from + "]" + "Chat [" + chat + "]");
}