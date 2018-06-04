// MazeWorldServer.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#pragma comment(lib,"ws2_32.lib")

struct Vector3D
{
	float X;
	float Y;
	float Z;
	Vector3D(float _X, float _Y, float _Z);
	Vector3D()=default;
};

Vector3D::Vector3D(float _X, float _Y, float _Z)
{
	X = _X;
	Y = _Y;
	Z = _Z;
}

struct PlayerInfomation
{
	CHAR Name[200];
	Vector3D Location;
	Vector3D Velocity;
	SOCKADDR Addr;
	int Life;
	bool operator==(const PlayerInfomation& PITO);
};

bool PlayerInfomation::operator==(const PlayerInfomation & PITO)
{
	return !strcmp(Name,PITO.Name);
}

void T_CheckConnection();
bool FindInListByName(std::list<PlayerInfomation> &TargetList, CHAR *TargetName);

using std::list;
list<PlayerInfomation>* Levels[1000] = { NULL };
std::mutex LogicLock;
int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err=WSAStartup(wVersionRequested, &wsaData);
	if (err)
	{
		printf("Error:WSAStartup Failed!\n");
		system("pause");
		exit(-1);
	}
	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1)
	{
		printf("Error:WSAStartup Failed! Version Error\n");
		WSACleanup();
		system("pause");
		return -1;
	}
	SOCKET sSocket = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(1703);
	bind(sSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	printf("Socket Init Success\n");
	std::thread T_CC(T_CheckConnection);
	T_CC.detach();
	while (1)
	{
		char Buffer[10240];
		SOCKADDR AddrTmp;
		int AddrLenTmp=sizeof(AddrTmp);
		int RErr= recvfrom(sSocket, Buffer, 10240, 0,&AddrTmp, &AddrLenTmp);
		if (RErr==SOCKET_ERROR)
		{
			RErr=WSAGetLastError();
			printf("WSA Error,Code:%d\n", RErr);
			system("pause");
			return -1;
		}
		//printf("Log:Receive a Message\n");
		LogicLock.lock();
		if (!Levels[*(INT16*)Buffer])
		{
			Levels[*(INT16*)Buffer] = new list<PlayerInfomation>();
			PlayerInfomation TmpInfomation;
			char* BufferIndex = Buffer;
			BufferIndex += 2;
			strcpy(TmpInfomation.Name, BufferIndex);
			BufferIndex += strlen(TmpInfomation.Name) +1;
			TmpInfomation.Location = Vector3D(*(float*)BufferIndex, *(float*)(BufferIndex + 4), *(float*)(BufferIndex + 8));
			BufferIndex += 12;
			TmpInfomation.Velocity = Vector3D(*(float*)BufferIndex, *(float*)(BufferIndex + 4), *(float*)(BufferIndex + 8));
			TmpInfomation.Addr = AddrTmp;
			TmpInfomation.Life = 5;
			Levels[*(INT16*)Buffer]->push_back(TmpInfomation);
			printf("Create New Flag And Pushed in an Element:");
			auto AAA = Buffer;
			while((long long)AAA<(long long)BufferIndex)
			{
				printf("%c", *AAA);
				AAA++;
			}
			printf("\n");
			INT16 Zero = 0;
			sendto(sSocket, (char*)&Zero, 2, 0, &TmpInfomation.Addr,sizeof(SOCKADDR));
		}
		else
		{
			char* BufferIndex = Buffer;
			BufferIndex += 2;
			if (!FindInListByName(*Levels[*(INT16*)Buffer], BufferIndex))
			{
				PlayerInfomation TmpInfomation;
				strcpy(TmpInfomation.Name, BufferIndex);
				//wcscpy(TmpInfomation.Name, (WCHAR*)BufferIndex);
				BufferIndex += strlen(TmpInfomation.Name) + 1;
				//BufferIndex += wcslen(TmpInfomation.Name) * 2 + 2;
				TmpInfomation.Location = Vector3D(*(float*)BufferIndex, *(float*)(BufferIndex + 4), *(float*)(BufferIndex + 8));
				BufferIndex += 12;
				TmpInfomation.Velocity = Vector3D(*(float*)BufferIndex, *(float*)(BufferIndex + 4), *(float*)(BufferIndex + 8));
				TmpInfomation.Life = 5;
				TmpInfomation.Addr = AddrTmp;
				Levels[*(INT16*)Buffer]->push_back(TmpInfomation);
				printf("Log:Pushed in an Element\n");
			}
			char DataToSent[10240] = {NULL};
			char* DTSIndex = DataToSent;
			*(INT16*)DTSIndex = (INT16)(Levels[*(INT16*)Buffer]->size()-1);
			DTSIndex += 2;
			BufferIndex = Buffer + 2;
			for (auto it = Levels[*(INT16*)Buffer]->begin(); it != Levels[*(INT16*)Buffer]->end(); ++it)
			{
				//printf("Loop1\n");
				if (!strcmp(it->Name,BufferIndex))
				{
					BufferIndex += strlen(BufferIndex) + 1;
					it->Life = 5;
					it->Location = Vector3D(*(float*)BufferIndex, *(float*)(BufferIndex + 4), *(float*)(BufferIndex + 8));
					BufferIndex += 12;
					it->Velocity = Vector3D(*(float*)BufferIndex, *(float*)(BufferIndex + 4), *(float*)(BufferIndex + 8));
					continue;
				}
				strcpy(DTSIndex, it->Name);
				//printf("%s\n", DTSIndex);
				//wcscpy((WCHAR*)DTSIndex, it->Name);
				DTSIndex += strlen(it->Name) +1;
				*(float*)DTSIndex = it->Location.X;
				*(float*)(DTSIndex+4) = it->Location.Y;
				*(float*)(DTSIndex+8) = it->Location.Z;
				DTSIndex += 12;
				*(float*)DTSIndex = it->Velocity.X;
				*(float*)(DTSIndex + 4) = it->Velocity.Y;
				*(float*)(DTSIndex + 8) = it->Velocity.Z;
				DTSIndex += 12;
			}
			sendto(sSocket, DataToSent, (int)((long long)DTSIndex - (long long)DataToSent), 0, &AddrTmp, sizeof(SOCKADDR));
			auto TTT = DataToSent;
			while((long long)TTT < (long long)DTSIndex)
			{
				printf("%c", *TTT);
				TTT++;
			}
			printf("\n");
		}
		LogicLock.unlock();
	}
    return 0;
}

void T_CheckConnection()
{
	while (1)
	{
		//printf("Loop2\n");
		LogicLock.lock();
		for (int i = 0; i < 1000; i++)
		{
			if (Levels[i])
			{
				for (auto it = Levels[i]->begin(); it != Levels[i]->end();)
				{
					it->Life--;
					if (it->Life <= 0)
					{
						//printf("Loop3\n");
						it=Levels[i]->erase(it);
						if (Levels[i]->size() <= 0)
						{
							delete Levels[i];
							Levels[i] = NULL;
							break;
						}
						continue;
					}
					++it;
				}
			}
		}
		LogicLock.unlock();
		Sleep(1000);
	}
}

bool FindInListByName(list<PlayerInfomation> &TargetList, CHAR *TargetName)
{
	for (auto it=TargetList.begin();it!=TargetList.end();++it)
	{
		//printf("Loop4\n");
		if (!strcmp(it->Name, TargetName))
			return true;
	}
	return false;
}

