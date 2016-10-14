// Fill out your copyright notice in the Description page of Project Settings.

#include "gameliftExample.h"
#include "TcpClient.h"
#include "PacketType.h"
#include "PeerManager.h"
#include <thread>
#include <assert.h>

#ifdef _win32
#pragma comment(lib, "ws2_32.lib")
#endif


// Sets default values
ATcpClient::ATcpClient() : RecvBuffer(BUF_SIZE), Socket(NULL), LoginId(-1)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATcpClient::BeginPlay()
{
	Super::BeginPlay();
	
	if (!Initialize())
	{
		Destroy();
	}

	TActorIterator<APeerManager> iter(GetWorld());

	PeerManager = *iter;
}

// Called every frame
void ATcpClient::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

bool ATcpClient::Connect(const FString& serverAddr, int port)
{
	struct hostent* host;
	struct sockaddr_in hostAddr;

	if ((host = gethostbyname(TCHAR_TO_UTF8(*serverAddr))) == 0)
		return false;

	memset(&hostAddr, 0, sizeof(hostAddr));
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr_list[0]))->s_addr;
	hostAddr.sin_port = htons(port);

	if (SOCKET_ERROR == ::connect(Socket, (struct sockaddr*)&hostAddr, sizeof(hostAddr)))
	{
		return false;
	}

	//u_long arg = 1;
	//ioctlsocket(mSocket, FIONBIO, &arg);

	/// nagle 알고리즘 끄기
	int opt = 1;
	setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int));

	return true;
}

void ATcpClient::LoginRequest(const FString& playerSessionId)
{
	if (LoginId > 0)
		return;

	::LoginRequest sendData;
	sprintf_s(sendData.mPlayerId, "%s", TCHAR_TO_UTF8(*playerSessionId));

	Send((const char*)&sendData, sizeof(::LoginRequest));
}

void ATcpClient::ChatRequest(const FString& chat)
{
	if (LoginId < 0)
		return;

	ChatBroadcastRequest sendData;
	sprintf_s(sendData.mPlayerId, "Player%d", LoginId);

	memcpy(sendData.mChat, TCHAR_TO_UTF8(*chat), strlen(TCHAR_TO_UTF8(*chat)));

	Send((const char*)&sendData, sizeof(ChatBroadcastRequest));
}

void ATcpClient::MoveRequest(float x, float y)
{
	if (LoginId < 0)
		return;

	::MoveRequest sendData;
	sendData.mPlayerIdx = LoginId;
	sendData.mPosX = x;
	sendData.mPosY = y;

	Send((const char*)&sendData, sizeof(::MoveRequest));
}

bool ATcpClient::Initialize()
{
#ifdef _WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;
#endif

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
		return false;

	auto t = std::thread([this]()
	{
		NetworkThread();
	});

	t.detach();

	return true;
}

bool ATcpClient::Send(const char* data, int length)
{
	int count = 0;
	while (count < length)
	{
		int n = ::send(Socket, data + count, length, 0);
		if (n == SOCKET_ERROR)
		{
			return false;
		}
		count += n;
		length -= n;
	}

	return true;
}

void ATcpClient::NetworkThread()
{
	while (true)
	{
		if (!IsValidLowLevel())
			break;

		char inBuf[4096] = { 0, };

		int n = ::recv(Socket, inBuf, 4096, 0);

		if (n < 1)
		{
			Sleep(0); ///< for cpu low-utilization
			continue;
		}


		if (!RecvBuffer.Write(inBuf, n))
		{
			/// 버퍼 꽉찼다. 
			assert(false);
		}

		ProcessPacket();
	}
}

void ATcpClient::ProcessPacket()
{
	/// 패킷을 파싱해서 완성되는 패킷이 있으면, 해당 콜백을 불러준다. 
	while (true)
	{
		PacketHeader header;

		if (false == RecvBuffer.Peek((char*)&header, sizeof(PacketHeader)))
			break;


		if (header.mSize > RecvBuffer.GetStoredSize())
			break;


		switch (header.mType)
		{
		case PKT_SC_LOGIN:
		{
			::LoginResult recvData;
			bool ret = RecvBuffer.Read((char*)&recvData, recvData.mSize);
			assert(ret);

			LoginId = 416;
		}
		break;

		case PKT_SC_CHAT:
		{
			ChatBroadcastResult recvData;
			bool ret = RecvBuffer.Read((char*)&recvData, recvData.mSize);
			assert(ret);

			PeerManager->PerformFunction(&APeerManager::ChatDraw, FString(UTF8_TO_TCHAR(recvData.mPlayerId)), FString(UTF8_TO_TCHAR(recvData.mChat)));
		}
		break;

		case PKT_SC_MOVE:
		{
			MoveBroadcastResult recvData;
			bool ret = RecvBuffer.Read((char*)&recvData, recvData.mSize);
			assert(ret);

			if (recvData.mPlayerIdx == LoginId)
				PeerManager->PerformFunction(&APeerManager::UpdateMe, recvData.mPosX, recvData.mPosY);
			else
				PeerManager->PerformFunction(&APeerManager::UpdatePeer, recvData.mPlayerIdx, recvData.mPosX, recvData.mPosY);
		}
		break;

		default:
			assert(false);
		}

	}
}