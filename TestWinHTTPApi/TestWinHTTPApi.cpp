#include <windows.h>
#include <stdio.h>
#include <Wininet.h>
#pragma comment(lib,"Wininet.lib")

int main()
{
	while (1)
	{
		system("cls");
		system("pause>nul");
		puts("任意键开始链接");
		auto HInternet= InternetOpen(TEXT(""), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		//auto hConnect = InternetConnect(HInternet, TEXT("https://api.imjad.cn/cloudmusic/?type=search&id=1&s=%E7%B4%85%E4%B8%80%E8%91%89"), INTERNET_DEFAULT_HTTPS_PORT, TEXT(""), TEXT(""), INTERNET_SERVICE_HTTP, 0, 0);
		//auto hReq =HttpOpenRequest(hConnect, TEXT("GET"), TEXT(""), HTTP_VERSION, TEXT(""), NULL, INTERNET_FLAG_SECURE, 0);
		//HttpSendRequest(hReq, NULL, 0, NULL, 0);
		auto hReq=InternetOpenUrl(HInternet, TEXT("https://api.imjad.cn/cloudmusic/?type=search&id=1&s=%E7%B4%85%E4%B8%80%E8%91%89"), 0, 0, INTERNET_FLAG_IGNORE_CERT_CN_INVALID| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, 0);
		/*if (!HttpSendRequest(hReq, NULL, 0, NULL, 0))
		{
			MessageBoxA(NULL, "Error", "Invalid Address", MB_OK);
			continue;
		}*/
		DWORD dwStatusCode, dwSizeOfStatusCode;
		BOOL bQueryInfo = HttpQueryInfo(hReq,
			HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
			&dwStatusCode,
			&dwSizeOfStatusCode,
			NULL);
		if (!bQueryInfo || dwStatusCode == 404)
		{
			MessageBoxA(NULL, "Error", "Invalid Address",MB_OK);
		}
		char buffer[10240];
		unsigned long dwRslt;
		
		InternetReadFile(hReq, buffer, 10240, &dwRslt); //读取网页内容；
		printf("%u,%s", dwRslt, buffer);
		system("pause>nul");
	}
}