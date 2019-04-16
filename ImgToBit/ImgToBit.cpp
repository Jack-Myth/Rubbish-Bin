#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char* argv[])
{
	int height, width,channels;
	unsigned char* imageData = stbi_load(argv[1], &width, &height, &channels, 0);
	if (!imageData)
	{
		printf("Error while loading Img:%s,Exit!",argv[1]);
		return -1;
	}
	FILE* fp;
	fopen_s(&fp, argv[2], "wb+");
	if (!fp)
	{
		printf("Error while open output file,Exit!");
		return -1;
	}
	char* bytes = new char[height*width / 8 + 1];
	int index = 0;
	int kb = 0;
	for (int y=0;y<height;y++)
	{
		for (int x=0;x<width;x++)
		{
			bytes[index] <<= 1;
			if (imageData[(y*width+x)*channels]>200)
				bytes[index]++;
			kb++;
			if (kb>=8)
			{
				index++;
				kb = 0;
			}
		}
	}
	fwrite(bytes, 1, height*width / 8, fp);
	fclose(fp);
	delete bytes;
	return 0;
}