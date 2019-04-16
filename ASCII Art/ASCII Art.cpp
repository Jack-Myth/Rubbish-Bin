#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "FreeImage/FreeImage.h"
#include <algorithm>
#pragma comment(lib,"FreeImage/FreeImage.lib")

WCHAR AArtPixelMap[] = { L' ',L'.',L'░',L'▒',L'▓',L'█' };

int main(int argc, char *argv[])
{
	bool Invert = false;
	bool DoubleWidth = false;
	for (int i=0;i<argc;i++)
	{
		if (!strcmp(argv[i],"-Invert"))
		{
			Invert = true;
		}
		else if (!strcmp(argv[i],"-DoubleWidth"))
		{
			DoubleWidth = true;
		}
	}
	WCHAR filename[1024] = {NULL};
	OPENFILENAME OpenFN = { NULL };
	OpenFN.lStructSize = sizeof(OPENFILENAME);
	//OpenFN.hwndOwner = hwnd;
	OpenFN.lpstrFilter = TEXT("图片文件\0*.jpg\0\0");
	OpenFN.nMaxFile = MAX_PATH;
	OpenFN.lpstrFile = filename;
	OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrTitle = TEXT("选择图片文件");
	if (GetOpenFileName(&OpenFN))
	{
		FIBITMAP* SourceImage= FreeImage_LoadU(FIF_JPEG, OpenFN.lpstrFile);
		FIBITMAP* TargetImage = NULL;
		//image.Load(OFN.lpstrFile);
		wprintf(TEXT("输入字符画的长宽(X,Y),或输入X自适应比例(x,Y)(X,x)，单个数字则等比缩放(以1.0为基准)\n"));
		float X,Y;
		char tmpS[1024];
		scanf("%[^,\n]", tmpS);
		if (getchar() == '\n')
		{
			sscanf(tmpS, "%f", &X);
			//等比缩放
			TargetImage=FreeImage_Rescale(SourceImage, FreeImage_GetWidth(SourceImage)*X, FreeImage_GetHeight(SourceImage)*X);
			//CxImageInstance.Resample(CxImageInstance.GetWidth()*X, CxImageInstance.GetHeight()*X);
		}
		else
		{
			//(X,Y)
			float Y;
			char *tmpc = tmpS;
			while (*tmpc == ' ')
			{
				tmpc++;
			}
			if (*tmpc=='x')
			{
				scanf("%f", &Y);
				X = Y / FreeImage_GetHeight(SourceImage)*FreeImage_GetWidth(SourceImage);
			}
			else
			{
				sscanf(tmpS, "%f",&X);
				scanf("%[^\n]", tmpS);
				tmpc = tmpS;
				while (*tmpc == ' ')
				{
					tmpc++;
				}
				if (*tmpc == 'x')
				{
					Y = X / FreeImage_GetWidth(SourceImage)*FreeImage_GetHeight(SourceImage);
				}
				else
				{
					scanf(tmpS, "%f", &Y);
				}
				TargetImage=FreeImage_Rescale(SourceImage, X, Y);
			}
		}
		unsigned int h = FreeImage_GetHeight(TargetImage);
		for (int y = h; y >=0; y--)
		{
			unsigned int w = FreeImage_GetWidth(TargetImage);
			for (int x = 0; x < w; x++)
			{
				RGBQUAD color;
				FreeImage_GetPixelColor(TargetImage,x,y,&color);
				if (Invert)
					if (DoubleWidth)
					{
						wprintf(TEXT("%c"), AArtPixelMap[sizeof(AArtPixelMap)-1-(int)(std::clamp<float>((color.rgbRed * 30 + color.rgbGreen * 59 + color.rgbBlue * 11 + 50) / 100 / 256.f,0,0.999f) * sizeof(AArtPixelMap))]);
						wprintf(TEXT("%c"), AArtPixelMap[sizeof(AArtPixelMap)-1-(int)(std::clamp<float>((color.rgbRed * 30 + color.rgbGreen * 59 + color.rgbBlue * 11 + 50) / 100 / 256.f,0,0.999f) * sizeof(AArtPixelMap))]);
					}
					else
						wprintf(TEXT("%c"), AArtPixelMap[sizeof(AArtPixelMap)-1-(int)(std::clamp<float>((color.rgbRed * 30 + color.rgbGreen * 59 + color.rgbBlue * 11 + 50) / 100 / 256.f,0,0.999f) * sizeof(AArtPixelMap))]);
				else
					if (DoubleWidth)
					{
						wprintf(TEXT("%c"), AArtPixelMap[(int)(std::clamp<float>((color.rgbRed * 30 + color.rgbGreen * 59 + color.rgbBlue * 11 + 50) / 100 / 256.f,0,0.999f) * sizeof(AArtPixelMap))]);
						wprintf(TEXT("%c"), AArtPixelMap[(int)(std::clamp<float>((color.rgbRed * 30 + color.rgbGreen * 59 + color.rgbBlue * 11 + 50) / 100 / 256.f, 0, 0.999f)* sizeof(AArtPixelMap))]);
					}
					else
						wprintf(TEXT("%c"), AArtPixelMap[(int)(std::clamp<float>((color.rgbRed * 30 + color.rgbGreen * 59 + color.rgbBlue * 11 + 50) / 100 / 256.f,0,0.999f) * sizeof(AArtPixelMap))]);
			}
			printf("\n");
		}
	}
	else
	{
		printf("Error :0x%X\n", CommDlgExtendedError());
	}
	system("pause");
}
