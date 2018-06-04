#include <stdio.h>
#include <Windows.h>  
#include <winsock.h> 
#include <thread>
#pragma comment(lib, "ws2_32.lib")  

WSADATA mWSAData;
SOCKET mSocket;
SOCKET ConnectedList[5] = { NULL };
SOCKADDR_IN ClientAddrList[5] = { NULL };
void WaitForAccept();
void WaitForRECV(SOCKET* S);
void FindAvailableSocketElement(SOCKET** socket, SOCKADDR_IN** Addr, int* place);
int main()
{
	if (WSAStartup(MAKEWORD(1, 1), &mWSAData))
	{
		printf("WSA Startup Failed");
		system("pause>nul");
		exit(-1);
	}
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (!mSocket)
	{
		printf("create Socket Failed");
		system("pause>nul");
		exit(-1);
	}
	SOCKADDR_IN Addr = { NULL };
	char AddrIPBuffer[20];
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(1703);
	Addr.sin_addr.S_un.S_addr = htons(INADDR_ANY);
	auto WaveData = new BYTE[32000];
	bind(mSocket, (SOCKADDR*)&Addr, sizeof(SOCKADDR));
	listen(mSocket, 5);
	std::thread(WaitForAccept).detach();
	printf("ServerStarted\n");
	while (1)
		Sleep(500);
}

void WaitForAccept()
{
	SOCKET* AvailableSocket = NULL;
	SOCKADDR_IN* Addrin = NULL;
	int ClientLen = sizeof(SOCKADDR);
	int place = 0;
	while (1)
	{
		FindAvailableSocketElement(&AvailableSocket, &Addrin,&place);
		if (AvailableSocket&&Addrin)
		{
			*AvailableSocket=accept(mSocket, (SOCKADDR*)Addrin, &ClientLen);
			if ((int)*AvailableSocket==-1)
			{
				printf("%d", WSAGetLastError());
			}
			printf("Get 1 Connected At Place%d\n", place);
			std::thread(WaitForRECV, AvailableSocket).detach();
		}
		else
			Sleep(1000);
	}
}

void WaitForRECV(SOCKET* S)
{
	char* ByteBuffer = new char[32000];
	while (1)
	{
		int byteReceived = 0;
		if (!(byteReceived=recv(*S, ByteBuffer, 32000, NULL)))
		{
			*S = NULL;
			printf("1 Connection has Closed\n");
			return;
		}
		printf("Package Received,Size:%d\n",byteReceived);
		for (int i=0;i<5;i++)
		{
			if (ConnectedList[i] && ConnectedList[i] != *S)
				send(ConnectedList[i], ByteBuffer, byteReceived, NULL);
		}
	}
}

void FindAvailableSocketElement(SOCKET** socket, SOCKADDR_IN** Addr,int* place)
{
	for (int i=0;i<5;i++)
		if (!ConnectedList[i])
		{
			*socket = &ConnectedList[i];
			*Addr = &ClientAddrList[i];
			*place = i;
			return;
		}
	*socket = NULL;
	*Addr = NULL;
	return;
}