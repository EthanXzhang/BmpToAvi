// RandomAviOutput.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <afx.h>
#include <atlimage.h>
#include "Vfw.h"
#include  <direct.h>  
#include  <stdio.h> 
#include <string>
//#include "afx.h"

void ImageToAVI(CString AviPath, CString ImgPath);
void CreateAVI(CImage img, CString name, int videoWidth, int videoHeight, int bpp);
//argv[1]=输入BMP文件夹，argv[2]=输出文件名
int main(int argc, char *argv[])
{
	printf("input BMP DirPath %s\n", argv[1]);
	printf("output AVI Path %s\n", argv[2]);
	if (argc < 2)
	{
		printf("check input argv\n");
		return 0;
	}
	char rootpath[MAX_PATH];
	_getcwd(rootpath, MAX_PATH);
	CString path = rootpath;
	CString input,output;
	if (argc = 3)
	{
		input = argv[1];
		output = argv[2];
	}
	else
	{
		input = rootpath;
		output = rootpath;
	}

	ImageToAVI(output, input);
	printf("BMP to AVI All Done! \n");
	return 0;
}
//======================================================================================
//
// 功能 : 将目录中的BMP图像无压缩封装为AVI视频
//
//======================================================================================
void ImageToAVI(CString AviPath, CString ImgPath)
{
	// 初始化AVI 库
	AVIFileInit();

	CFileFind finder;
	CString ImgDir=ImgPath+"\\*.bmp";
	BOOL bFind = finder.FindFile(ImgDir);
	//遍历文件夹路径中所有的*.bmp文件
	while (bFind)
	{
		bFind = finder.FindNextFile();
		if (!finder.IsDots() && !finder.IsDirectory())//若不为空且非文件夹
		{
			// 获取图像文件绝对路径
			CString str = finder.GetFilePath();
			CString title = finder.GetFileTitle();
			CString name = AviPath + "\\"+title+".avi";
			//拷贝bmp图片文件名作为生成avi文件名
			//
			// Open Image and get image info & data
			//使用CImage读取图片信息
			CImage img;		
			if (FAILED(img.Load(str)))
			{
				TRACE("Warning : fail to load file %s!!!\n", str);
				continue;
			}
			int w, h,bpp;
			w = img.GetWidth();
			h = img.GetHeight();
			bpp = img.GetBPP();
			//img.Destroy();
			//释放CImage，使用FILE读写图片数据
			//std::string s = CT2A(str.GetBuffer(0));
			//FILE *fp;
			//fopen_s(&fp,s.c_str(), "rb");
			CreateAVI(img,name,w,h,bpp);//创建avi视频函数
			printf("BMP to AVI Done and Go Next! \n");
			
		}
	}

	AVIFileExit();

}
void CreateAVI(CImage img,CString name, int videoWidth, int videoHeight, int bpp)
{
	//
	// Create AVI file
	//
	PAVIFILE pAviFile = NULL;
	HRESULT hr = AVIFileOpen(&pAviFile, name, OF_WRITE | OF_CREATE, NULL);
	if (0 != hr)
	{
		return;
	}

	//
	// Create AVI video stream
	//
	AVISTREAMINFO strhdr;
	memset(&strhdr, 0, sizeof(strhdr));

	strhdr.fccType = streamtypeVIDEO;
	strhdr.fccHandler = mmioFOURCC('X', 'V', 'I', 'D');
	strhdr.dwScale = 1;
	strhdr.dwRate = 0.1;
	//设置帧速 1秒/帧

	UINT pitch = (videoWidth * bpp + 31) / 32 * 4;
	UINT biSizeImage = (videoWidth * bpp + 31) / 32 * 4 * videoHeight;
	strhdr.dwSuggestedBufferSize = biSizeImage;

	SetRect(&strhdr.rcFrame, 0, 0, videoWidth, videoHeight);

	// And create the stream;
	PAVISTREAM pAviStream = NULL;
	hr = AVIFileCreateStream(pAviFile, &pAviStream, &strhdr);
	if (0 != hr)
	{
		AVIFileRelease(pAviFile);
		pAviFile = NULL;
		return;
	}

	//
	// Set stream format
	//
	BITMAPINFOHEADER bih;
	memset(&bih, 0, sizeof(BITMAPINFOHEADER));

	bih.biBitCount = bpp;
	bih.biClrImportant = 0;
	bih.biClrUsed = 0;
	bih.biCompression = BI_RGB;
	bih.biPlanes = 1;
	bih.biSize = 40;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biWidth = videoWidth;
	bih.biHeight = videoHeight;
	bih.biSizeImage = biSizeImage;

	hr = AVIStreamSetFormat(pAviStream, 0, &bih, sizeof(bih));

	if (0 != hr)
	{

		AVIStreamClose(pAviStream);
		pAviStream = NULL;

		AVIFileRelease(pAviFile);
		pAviFile = NULL;
	}

	// 图像数据缓冲区
	BYTE * pData = new BYTE[biSizeImage];
	if (pData)
	{
		memset(pData, 0, biSizeImage);
	}

	//
	// 读取图像数据，更新AVI视频帧
	//
	if (pData)
	{
		for (int nFrames = 0; nFrames < 1; nFrames++)//同一图像拷贝10帧，由图像生成10秒avi视频
		{
			for (int i = 0; i < videoHeight; i++)
			{
				for (int j = 0; j < videoWidth; j++)
				{
					COLORREF clr = img.GetPixel(j, videoHeight - 1 - i);//(j,i)  
					pData[i * pitch + j * (bpp / 8) + 0] = GetBValue(clr);
					pData[i * pitch + j * (bpp / 8) + 1] = GetGValue(clr);
					pData[i * pitch + j * (bpp / 8) + 2] = GetRValue(clr);

				}
			}
			hr = AVIStreamWrite(pAviStream, nFrames, 1, pData, biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
		}
	}


	if (pData != NULL)
	{
		delete[] pData;
		pData = NULL;
	}

	if (pAviStream != NULL)
	{
		AVIStreamClose(pAviStream);
		pAviStream = NULL;
	}

	if (pAviFile != NULL)
	{
		AVIFileRelease(pAviFile);
		pAviFile = NULL;
	}
}