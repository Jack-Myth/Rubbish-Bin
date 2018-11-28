//Created by JackMyth At 2018-4-30
#define _CRT_SECURE_NO_WARNINGS
#include <Winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <io.h>
#include <algorithm>
#include <ws2def.h>
#include <ws2ipdef.h>
#pragma comment(lib, "ws2_32.lib")

#define USE_IPV6 1

WSADATA wsaData;

struct
{
	std::string WebHome;
	std::vector<std::string> MainPageName;
	int Port;
	std::map<std::string, std::string> MIME;
} GlobalSettings;
#if USE_IPV6
void OnRequestArrived(SOCKET ConnectionSocket, SOCKADDR_IN6  ClientAddr, int AddSize);
#else
void OnRequestArrived(SOCKET ConnectionSocket, SOCKADDR_IN  ClientAddr, int AddSize);
#endif
void LoadProfileFromFile(std::string ProfileName);
int main(int argv,char* argc[])
{
	std::string ProfileName;
	{
		char WorkingPath[1024];
		GetCurrentDirectoryA(1024, WorkingPath);
		ProfileName = std::string(WorkingPath) + "\\Profile.ini";
	}
	//Process Args.
	for (int i = 0; i < argv; i++)
	{
		if (std::string(argc[i]) == "-c")
		{
			i++;
			ProfileName = std::string(argc[i]);
		}
	}
	LoadProfileFromFile(ProfileName);
	if (WSAStartup(MAKEWORD(1, 1), &wsaData))
	{
		printf("WSA Startup Failed, Exit...\n");
		exit(-1);
	}
#if USE_IPV6
	SOCKET MainSocket = socket(PF_INET6, SOCK_STREAM, 0);
	SOCKADDR_IN6 ServerAddr = { 0 };
	ServerAddr.sin6_addr = IN6ADDR_ANY_INIT;
	ServerAddr.sin6_family=PF_INET6;
	ServerAddr.sin6_port = htons(GlobalSettings.Port);
	if (bind(MainSocket, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR_IN6)))
#else
	SOCKET MainSocket = socket(AF_INET, SOCK_STREAM, 0)
	SOCKADDR_IN ServerAddr;
	ServerAddr.sin_addr.S_un.S_addr = htonl(ADDR_ANY);
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(GlobalSettings.Port);
	if (bind(MainSocket, (SOCKADDR*)&ServerAddr, sizeof(SOCKADDR)))
#endif
	{
		printf("Socket Bind Failed, Please make sure your 80 port is not in use.\n");
		exit(-1);
	}
	if (listen(MainSocket, 5))
	{
		printf("Socket Listen Failed, Exit...\n");
		exit(-1);
	}
	while (1)
	{
#if USE_IPV6
		SOCKADDR_IN6 ClientAddr;
		int AddrSize = sizeof(SOCKADDR_IN6);
#else
		SOCKADDR_IN ClientAddr;
		int AddrSize = sizeof(SOCKADDR);
#endif
		SOCKET CurConnection = accept(MainSocket, (SOCKADDR*)&ClientAddr, &AddrSize);
		std::thread(&OnRequestArrived, CurConnection, ClientAddr, AddrSize).detach();
	}
	return 0;
}

void LoadProfileFromFile(std::string ProfileName)
{
	//WebHome
	char WorkingPath[1024];
	GetCurrentDirectoryA(1024,WorkingPath);
	GetPrivateProfileStringA("Config", "WebHome", WorkingPath, WorkingPath, 1024, ProfileName.c_str());
	GlobalSettings.WebHome = std::string(WorkingPath);
	//Port
	GlobalSettings.Port= GetPrivateProfileIntA("Config", "Port", 80, ProfileName.c_str());
	//Main Page
	std::string MainPg;
	MainPg.resize(4096);
	GetPrivateProfileStringA("Config", "HomePages", "Index.html:Index.htm", MainPg.data(), 4096, ProfileName.c_str());
	if (!ProfileName.empty())
	{
		int LastIndex = 0, CurIndex = 0;
		for (; MainPg[CurIndex]; CurIndex++)
		{
			if (MainPg[CurIndex] == ':')
			{
				GlobalSettings.MainPageName.push_back(MainPg.substr(LastIndex, CurIndex - LastIndex));
				LastIndex = CurIndex + 1;
			}
		}
		GlobalSettings.MainPageName.push_back(MainPg.substr(LastIndex, CurIndex - LastIndex));
	}
	//Load MIME
	std::vector<char> AllMIME;
	int AllMIMESize = 1024;
	AllMIME.resize(AllMIMESize);
	while ((int)GetPrivateProfileStringA("MIME", NULL, NULL, AllMIME.data(), AllMIMESize, ProfileName.c_str()) >= AllMIMESize - 2)
		AllMIME.resize(AllMIMESize *= 2);
	if (AllMIME[0] == '\0')
	{
		printf("Invalid MIME Config! Exit...\n");
		exit(-1);
	}
	for (int i=0;AllMIME[i];i++)
	{
		std::string tmpExtension = AllMIME.data() + i;
		char MIMEType[1024];
		GetPrivateProfileStringA("MIME",tmpExtension.c_str(), NULL, MIMEType, 1024, ProfileName.c_str());
		GlobalSettings.MIME.insert_or_assign(tmpExtension, std::string(MIMEType));
		i += tmpExtension.length();
	}
}

std::string GetFileExtension(std::string FileName)
{
	for (auto it=FileName.length()-1;it>=0;it--)
	{
		if (FileName[it] == '/')
			return std::string();
		else if (FileName[it] == '.')
			return FileName.substr(it+1,FileName.length()-it);
	}
	return std::string();
}

//Check Is File extension have an avaliable MIME.
//True if so. Otherwise False.
//ReturnMIME will store the MIME for the file extension.
inline bool CheckMIME(std::string FileExtension,std::string* ReturnMIME = nullptr)
{
	transform(FileExtension.begin(), FileExtension.end(), FileExtension.begin(), tolower);
	auto it = GlobalSettings.MIME.find(FileExtension);
	if (it == GlobalSettings.MIME.end())
		return false;
	if (ReturnMIME)
		*ReturnMIME = it->second;
	return true;
}
#if USE_IPV6
void OnRequestArrived(SOCKET ConnectionSocket, SOCKADDR_IN6  ClientAddr, int AddSize)
#else
void OnRequestArrived(SOCKET ConnectionSocket, SOCKADDR_IN  ClientAddr, int AddSize)
#endif
{
	struct
	{
		std::string HTTPVersion = "HTTP/1.1";
		std::string StatusCode = "500 Internal Server Error";
		std::map<std::string, std::string> DomainCollection;
	} ResponseHeader;
	std::vector<char> HTMLData;
	for (;;)
	{
		char DataBlock[1024];
		int DataRecvd = recv(ConnectionSocket, DataBlock, 1024, 0);
		if (DataRecvd ==SOCKET_ERROR||!DataRecvd)
		{
			if (HTMLData.size() == 0)
			{
				closesocket(ConnectionSocket);
				return;
			}
			else
				break;
		}
		HTMLData.assign(DataBlock, DataBlock + DataRecvd);
		if (DataRecvd<1024)
			break;
	}
	std::istringstream HTMLStream(HTMLData.data(), HTMLData.size());
	std::string TmpRequestHeader;
	std::getline(HTMLStream, TmpRequestHeader);
	std::string HTMLMethod, RequestTarget, HTTPVersion;
	std::istringstream(TmpRequestHeader.c_str()) >> HTMLMethod >> RequestTarget >> HTTPVersion;
	std::map<std::string, std::string> DomainCollection;
	std::vector<char> ContentData;
	for (;;)
	{
		std::getline(HTMLStream, TmpRequestHeader);
		TmpRequestHeader = TmpRequestHeader.substr(0, TmpRequestHeader.length() - 1);
		if (TmpRequestHeader.empty())
			break;
		std::istringstream tmpStrStream(TmpRequestHeader.c_str());
		std::string Domain;
		std::string Value;
		std::getline(tmpStrStream, Domain, ':');
		tmpStrStream >> Value;
		DomainCollection.insert_or_assign(Domain, Value);
	}
	if (RequestTarget[0] != '/')
	{
		printf("Invalid Request Target, Abandon!\n");
		closesocket(ConnectionSocket);
	}
	printf((std::string("Request For ") + RequestTarget+"\n").c_str());
	if (int offset = RequestTarget.find("?"); offset != std::string::npos)
		RequestTarget = RequestTarget.substr(0, offset);
	if (_access((GlobalSettings.WebHome + RequestTarget).c_str(), 0))//Check Exist Permission 
		ResponseHeader.StatusCode = "404 Not Found";
	else
	{
		std::string MIMEType;
		if (!_access((GlobalSettings.WebHome + RequestTarget).c_str(), 4))    //Check Read Permission )
		{
			if (GetFileAttributesA((GlobalSettings.WebHome + RequestTarget).c_str())&FILE_ATTRIBUTE_DIRECTORY)	//IsDirectory?
			{
				ResponseHeader.StatusCode = "403 Forbidden";
				for (auto it = GlobalSettings.MainPageName.begin(); it < GlobalSettings.MainPageName.end(); ++it)  //Try Find MainPage(Such as Index.html)
				{
					if (!CheckMIME(GetFileExtension(GlobalSettings.WebHome + RequestTarget + *it), &MIMEType))
						break;
					if (!_access((GlobalSettings.WebHome + RequestTarget + "/" + *it).c_str(), 4))
					{
						ResponseHeader.DomainCollection.insert_or_assign(std::string("Content-Type"), MIMEType);
						FILE* pFile;
						pFile = fopen((GlobalSettings.WebHome + RequestTarget+*it).c_str(), "rb");
						fseek(pFile, 0, SEEK_END);
						int FileSize;
						FileSize = ftell(pFile);
						fseek(pFile, 0, SEEK_SET);
						ResponseHeader.DomainCollection.insert_or_assign(std::string("Content-Length"), std::to_string(FileSize));
						ContentData.resize(FileSize);
						fread(ContentData.data(), FileSize, 1, pFile);
						fclose(pFile);
						ResponseHeader.StatusCode = "200 OK";
						break;
					}
				}
			}
			else if (CheckMIME(GetFileExtension(GlobalSettings.WebHome + RequestTarget), &MIMEType))//Check MIME
			{
				ResponseHeader.DomainCollection.insert_or_assign(std::string("Content-Type"), MIMEType);
				FILE* pFile;
				pFile = fopen((GlobalSettings.WebHome + RequestTarget).c_str(), "rb");
				fseek(pFile, 0, SEEK_END);
				int FileSize;
				FileSize = ftell(pFile);
				fseek(pFile, 0, SEEK_SET);
				ResponseHeader.DomainCollection.insert_or_assign(std::string("Content-Length"), std::to_string(FileSize));
				ContentData.resize(FileSize);
				fread(ContentData.data(), FileSize, 1, pFile);
				fclose(pFile);
				ResponseHeader.StatusCode = "200 OK";
			}
			else
				ResponseHeader.StatusCode = "403 Forbidden";
		} 
		else
			ResponseHeader.StatusCode = "403 Forbidden";
	}
	ResponseHeader.DomainCollection.insert_or_assign("Server", "Hello MyHTTPServer");
	//~~~~~~~~~~~~~~~~~~~~~~~
	//Construct Response Package
	//~~~~~~~~~~~~~~~~~~~~~~~
	std::string HeaderLine;
	HeaderLine = ResponseHeader.HTTPVersion + " " + ResponseHeader.StatusCode + "\r\n";
	send(ConnectionSocket, HeaderLine.c_str(), HeaderLine.length(), 0);
	for (auto it = ResponseHeader.DomainCollection.begin(); it != ResponseHeader.DomainCollection.end(); ++it)
	{
		HeaderLine = it->first + ":" + it->second + "\r\n";
		send(ConnectionSocket, HeaderLine.c_str(), HeaderLine.length(), 0);
	}
	HeaderLine = "\r\n";
	send(ConnectionSocket, HeaderLine.c_str(), HeaderLine.length(), 0);
	send(ConnectionSocket, ContentData.data(), ContentData.size(), 0);
	closesocket(ConnectionSocket);
}