

#include "stdafx.h"

#include "afxdialogex.h"
#include<tchar.h>
#include <opencv2/opencv.hpp>
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"

#include <Vfw.h>
#include <fstream>
#include<iostream>
#include<vector>
#include <Windows.h> 
#include <iterator>
#include <string>
#include <io.h>

#include <Vfw.h>

using namespace std;
using namespace cv;
#pragma comment (lib, "vfw32.lib")

#pragma warning(disable:4996)

#include "SensorIniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern class cparamlist param_buffer;

#pragma comment (lib,".\\DTCCM2_SDK\\dtccm2.lib")


// 通用信息处理函数，用来描述设备信息的
void msg(LPCSTR lpszFmt, ...)
{
	static CMFCApplication1Dlg* pDlg = NULL;

	char *szTmp = (char*)malloc(512);

	if (szTmp != NULL)
	{
		va_list argList;
		va_start(argList, lpszFmt);
		_vsnprintf(szTmp, 512, lpszFmt, argList);
		va_end(argList);

		if (pDlg == NULL)
		{
			pDlg = (CMFCApplication1Dlg*)AfxGetMainWnd();
		}

		if (pDlg != NULL)
		{
			if (!PostMessage(pDlg->GetSafeHwnd(), WM_MSG, (WPARAM)szTmp, 0))
			{
				free(szTmp);
			}
		}
		else
		{
			free(szTmp);
		}
	}
}

UINT __stdcall RemoteDebugThread(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	pDlg->WorkProc();
	return 0;
}





UINT __stdcall show_self(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	BYTE *pInData = param_buffer.pInData;
	ULONG uDataSize = param_buffer.uDataSize;
	BYTE *pOutBuffer = param_buffer.pOutBuffer;
	int iWidth = param_buffer.iWidth;
	int iHeight = param_buffer.iHeight;

	pDlg->Display_image_byself(pInData, uDataSize, pOutBuffer, iWidth, iHeight);

	return 0;
}

UINT __stdcall slow_Thread1(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	BYTE *pInData = param_buffer.pInData;
	ULONG uDataSize = param_buffer.uDataSize;
	BYTE *pOutBuffer = param_buffer.pOutBuffer;
	int iWidth = param_buffer.iWidth;
	int iHeight = param_buffer.iHeight;

	pDlg->Display_slow_data(pInData, uDataSize, pOutBuffer, iWidth, iHeight);

	return 0;
}

UINT __stdcall save_Thread(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	pDlg->save_raw_data();
	return 0;
}

UINT __stdcall offline_data_proc(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	pDlg->load_and_proc();
	return 0;
}

void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
