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

extern void msg(LPCSTR lpszFmt, ...);
extern UINT __stdcall RemoteDebugThread(LPVOID param);
extern UINT __stdcall show_self(LPVOID param);
extern UINT __stdcall slow_Thread1(LPVOID param);
extern UINT __stdcall slow_disp_Thread(LPVOID param);
extern UINT __stdcall save_Thread(LPVOID param);
extern UINT __stdcall offline_data_proc(LPVOID param);
extern void getFiles(string path, vector<string>& files);

#pragma comment (lib,".\\DTCCM2_SDK\\dtccm2.lib")

void CMFCApplication1Dlg::Display_image_from_camera(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight)
{

	int i, j;
	int iPixels = iWidth * iHeight;
	USHORT *pOut = (USHORT*)pOutBuffer;

	// 此时，获得是预先累加过的图像数据，一次800帧        
	// 每个字节保存了两个像素的累加和，4bit一个，由于进位的关系，4bit最多只能保存15次累加和信息；

	// 以下7个指针用于指向7个数据块，每个数据块分别保存6个15帧的累加和信息和1个10帧的累加和信息，一共100帧；
	// 800帧就有8*7个这样的数据块；
	BYTE *p0, *p1, *p2, *p3, *p4, *p5, *p6;
	int iAccDataSize = iPixels / 2;

	// 计算第一个100帧的积分数据
	p0 = pInData;
	p1 = p0 + iAccDataSize;
	p2 = p0 + 2 * iAccDataSize;
	p3 = p0 + 3 * iAccDataSize;
	p4 = p0 + 4 * iAccDataSize;
	p5 = p0 + 5 * iAccDataSize;
	p6 = p0 + 6 * iAccDataSize;

	for (j = 0; j < iPixels; j += 2)
	{
		pOut[j] = (*p0 & 0xf) + (*p1 & 0xf) + (*p2 & 0xf) + (*p3 & 0xf) + (*p4 & 0xf) + (*p5 & 0xf) + (*p6 & 0xf);
		pOut[j + 1] = ((*p0 >> 4) & 0xf) + ((*p1 >> 4) & 0xf) + ((*p2 >> 4) & 0xf) + ((*p3 >> 4) & 0xf) + ((*p4 >> 4) & 0xf) + ((*p5 >> 4) & 0xf) + ((*p6 >> 4) & 0xf);
		p0++;
		p1++;
		p2++;
		p3++;
		p4++;
		p5++;
		p6++;
	}

	// 累加7个100帧的积分数据
	for (i = 1; i < m_uFrameCusum / 100; i++)
	{
		// 每次计算100帧的积分数据
		p0 = pInData + (i*iAccDataSize * 7);
		p1 = p0 + iAccDataSize;
		p2 = p0 + 2 * iAccDataSize;
		p3 = p0 + 3 * iAccDataSize;
		p4 = p0 + 4 * iAccDataSize;
		p5 = p0 + 5 * iAccDataSize;
		p6 = p0 + 6 * iAccDataSize;

		if ((m_uFrameCusum / 100) == 1)
			break;

		for (j = 0; j < iPixels; j += 2)
		{
			pOut[j] += (*p0 & 0xf) + (*p1 & 0xf) + (*p2 & 0xf) + (*p3 & 0xf) + (*p4 & 0xf) + (*p5 & 0xf) + (*p6 & 0xf);
			pOut[j + 1] += ((*p0 >> 4) & 0xf) + ((*p1 >> 4) & 0xf) + ((*p2 >> 4) & 0xf) + ((*p3 >> 4) & 0xf) + ((*p4 >> 4) & 0xf) + ((*p5 >> 4) & 0xf) + ((*p6 >> 4) & 0xf);
			p0++;
			p1++;
			p2++;
			p3++;
			p4++;
			p5++;
			p6++;
		}
	}
	// 把原先的drawImage删了，改成opencv里的图像显示，但还没测试
	pOut = (USHORT*)pOutBuffer;
	Mat img(Size(iWidth, iHeight), CV_8UC1);
	uchar *datain = img.data;
	for (i = 0; i < iPixels; i++)
	{
		datain[i] = (pOut[i] * 2 > 255) ? 255 : (pOut[i] * 2 & 0xff);
	}
	resize(img, img, Size(rt_rect.Width(), rt_rect.Height()));
	flip(img, img, 0);
	imshow("RT", img);
	waitKey(5);
	Sleep(5);

}
void CMFCApplication1Dlg::Display_image_byself(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight)
{

	int iPixels = iWidth * iHeight;
	USHORT *pOut = (USHORT*)pOutBuffer;


	int itemp = 0;
	int iframe = 0;
	int frame;


	uint irow = 0;
	uint itime = 0;
	uchar q1 = 1;
	uchar q2 = 2;
	uchar q3 = 4;
	uchar q4 = 8;
	uchar q5 = 16;
	uchar q6 = 32;
	uchar q7 = 64;
	uchar q8 = 128;

	int length = FRAME_CUSUM_CNT * 50 * 250;
	uchar *pulse = new uchar[FRAME_CUSUM_CNT * 250 * 50];

	Mat img(Size(iWidth, iHeight), CV_8UC1);
	Mat img1(Size(rt_rect.Width(), rt_rect.Height()), CV_8UC1);

	while (m_rawproc) {
		WaitForSingleObject(m_Event, INFINITE);

		WaitForSingleObject(m_Mutex, INFINITE);
		memcpy(pulse, raw_data, FRAME_CUSUM_CNT * 250 * 50);
		ReleaseMutex(m_Mutex);


		memset(img.data, 0x0, iHeight*iWidth);
		memset(img1.data, 0x0, rt_rect.Width()* rt_rect.Height());
		uchar *dbuffer = img.data;

		for (iframe = 0; iframe < FRAME_CUSUM_CNT; iframe++)
		{
			frame = iframe;
			for (irow = 0; irow < 250; irow++)
			{
				for (itime = 0; itime < 50; itime++)//每行52个字节
				{

					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q1)
					{
						dbuffer[8 * 50 * irow + 8 * itime]++;
					}
					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q2)
					{
						dbuffer[8 * 50 * irow + 8 * itime + 1]++;
					}
					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q3)
					{
						dbuffer[8 * 50 * irow + 8 * itime + 2]++;
					}
					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q4)
					{
						dbuffer[8 * 50 * irow + 8 * itime + 3]++;
					}
					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q5)
					{
						dbuffer[8 * 50 * irow + 8 * itime + 4]++;
					}
					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q6)
					{
						dbuffer[8 * 50 * irow + 8 * itime + 5]++;
					}
					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q7)
					{
						dbuffer[8 * 50 * irow + 8 * itime + 6]++;
					}
					if (pulse[frame * 50 * 250 + irow * 50 + itime] & q8)
					{
						dbuffer[8 * 50 * irow + 8 * itime + 7]++;
					}
				}
			}
		}
		resize(img, img1, Size(rt_rect.Width(), rt_rect.Height()));
		flip(img1, img1, 0);
		equalizeHist(img1, img1);
		imshow("RT", img1);
		waitKey(5);
		Sleep(5);
	}
}
void CMFCApplication1Dlg::Display_slow_data(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight)
{
	int height = 250;
	int width = 400;

	int length = 250 * 400 * FRAME_CUSUM_CNT / 8;
	uchar q1 = 1;
	uchar q2 = 2;
	uchar q3 = 4;
	uchar q4 = 8;
	uchar q5 = 16;
	uchar q6 = 32;
	uchar q7 = 64;
	uchar q8 = 128;
	int iPixels = iHeight* iWidth;


	uchar * pulse = new uchar[FRAME_CUSUM_CNT * 250 * 50];
	uint* interval = new uint[250 * 400];
	uint* label = new uint[250 * 400];
	uint* gray_first = new uint[250 * 400];
	uchar* gray = new uchar[250 * 400];
	uchar* bit_img = new uchar[250 * 400];


	for (int jj = 0; jj < 250 * 40 / 8; jj++)
	{
		label[jj] = 0x00;
	}


	uint itemp = 0;
	uint iframe = 0;
	uint frame;

	uint irow = 0;
	uint itime = 0;

	while (m_slow_proc) {
		WaitForSingleObject(m_slow_data_Event, INFINITE);

		for (int ii = 0; ii < 5; ii++) {
			memcpy(pulse, temp_data_buffer+ ii*FRAME_CUSUM_CNT * 250 * 50, FRAME_CUSUM_CNT * 250 * 50);

			for (iframe = 0; iframe < FRAME_CUSUM_CNT; iframe++)
			{
				frame = iframe;
				memset(bit_img, 0x0, iWidth*iHeight);
				for (irow = 0; irow < 250; irow++)
				{
					for (itime = 0; itime < 50; itime++)//每行52个字节
					{

						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q1)
						{
							interval[8 * 50 * irow + 8 * itime] = frame - label[8 * 50 * irow + 8 * itime];
							label[8 * 50 * irow + 8 * itime] = frame;
							bit_img[8 * 50 * irow + 8 * itime] = 0xff;
						}
						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q2)
						{
							interval[8 * 50 * irow + 8 * itime + 1] = frame - label[8 * 50 * irow + 8 * itime + 1];
							label[8 * 50 * irow + 8 * itime + 1] = frame;
							bit_img[8 * 50 * irow + 8 * itime + 1] = 0xff;
						}
						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q3)
						{
							interval[8 * 50 * irow + 8 * itime + 2] = frame - label[8 * 50 * irow + 8 * itime + 2];
							label[8 * 50 * irow + 8 * itime + 2] = frame;
							bit_img[8 * 50 * irow + 8 * itime + 2] = 0xff;
						}
						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q4)
						{
							interval[8 * 50 * irow + 8 * itime + 3] = frame - label[8 * 50 * irow + 8 * itime + 3];
							label[8 * 50 * irow + 8 * itime + 3] = frame;
							bit_img[8 * 50 * irow + 8 * itime + 3] = 0xff;
						}
						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q5)
						{
							interval[8 * 50 * irow + 8 * itime + 4] = frame - label[8 * 50 * irow + 8 * itime + 4];
							label[8 * 50 * irow + 8 * itime + 4] = frame;
							bit_img[8 * 50 * irow + 8 * itime + 4] = 0xff;
						}
						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q6)
						{
							interval[8 * 50 * irow + 8 * itime + 5] = frame - label[8 * 50 * irow + 8 * itime + 5];
							label[8 * 50 * irow + 8 * itime + 5] = frame;
							bit_img[8 * 50 * irow + 8 * itime + 5] = 0xff;
						}
						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q7)
						{
							interval[8 * 50 * irow + 8 * itime + 6] = frame - label[8 * 50 * irow + 8 * itime + 6];
							label[8 * 50 * irow + 8 * itime + 6] = frame;
							bit_img[8 * 50 * irow + 8 * itime + 6] = 0xff;
						}
						if (pulse[frame * 50 * 250 + irow * 50 + itime] & q8)
						{
							interval[8 * 50 * irow + 8 * itime + 7] = frame - label[8 * 50 * irow + 8 * itime + 7];
							label[8 * 50 * irow + 8 * itime + 7] = frame;
							bit_img[8 * 50 * irow + 8 * itime + 7] = 0xff;
						}
					}
				}
				for (int mm = 0; mm < 250 * 400; mm++)
				{
					if (interval[mm] != 0)
					{
						gray_first[mm] = (1600 / interval[mm] > 255) ? 255 : ((1600 / interval[mm]) & 0xff);;
						gray[mm] = gray_first[mm];
					}
					if (interval[mm] == 0)
					{
						gray[mm] = 0;
					}
				}

				memcpy(temp_disp_data_buffer + f_disp_location * 5 * FRAME_CUSUM_CNT * 250 * 400 +
					ii*FRAME_CUSUM_CNT * 400 * 250 + iframe * 400 * 250, 
					gray, 
					FRAME_CUSUM_CNT * 400 * 250);
				memcpy(temp_disp_bit_buffer + f_disp_location * 5 * FRAME_CUSUM_CNT * 250 * 400 +
					ii*FRAME_CUSUM_CNT * 400 * 250 + iframe * 400 * 250,
					bit_img,
					FRAME_CUSUM_CNT * 400 * 250);
			}


		}
		if (m_hThread_slow == INVALID_HANDLE_VALUE)
			m_hThread_slow = (HANDLE)_beginthreadex(NULL, 0, &slow_disp_Thread, this, 0, 0);
	
		if (f_disp_location == 0)
			f_disp_location = 1;
		else
			f_disp_location = 0;
	}
	

	delete[] pulse;
	delete[] interval;
	delete[] label;
	delete[] gray_first;
	delete[] gray;
	delete[] bit_img;
}
void CMFCApplication1Dlg::slow_disp()
{

	int iWidth = WIDTH;
	int iHeight = HEIGHT;
	int offset = 5 * FRAME_CUSUM_CNT * 400 * 250;
	Mat img(Size(iWidth, iHeight), CV_8UC1);
	uchar *data = img.data;
	;
	while (m_display_slow) {
		frame_offset++;
		memcpy(data,
			temp_disp_data_buffer + offset * f_disp_location + (frame_offset*slow_rate + 150)*iWidth*iHeight,
			iWidth*iHeight);

		resize(img, img, Size(sl_rect.Width(), sl_rect.Height()));
		flip(img, img, 0);
		imshow("SL", img);
		waitKey(10);

		Mat bit_img(Size(iWidth, iHeight), CV_8UC1);
		data = bit_img.data;
		memset(data, 0x0, iHeight*iWidth);

		memcpy(data,
			temp_disp_bit_buffer + offset * f_disp_location + (frame_offset*slow_rate + 150)*iWidth*iHeight,
			iWidth*iHeight);

		resize(bit_img, bit_img, Size(pl_rect.Width(), pl_rect.Height()));
		flip(bit_img, bit_img, 0);
		imshow("PL", bit_img);
		waitKey(10);
	}
	
	
}

void CMFCApplication1Dlg::Display_slow(int iWidth, int iHeight)
{
	Mat img(Size(iWidth, iHeight), CV_8UC1);
	uchar *data = img.data;
	memset(data, 0x0, iHeight*iWidth);

	memcpy(data, slowdata + (f_cachebit_count*slow_rate + 150)*iWidth*iHeight, iWidth*iHeight);

	resize(img, img, Size(sl_rect.Width(), sl_rect.Height()));
	flip(img, img, 0);
	imshow("SL", img);


	Mat bit_img(Size(iWidth, iHeight), CV_8UC1);
	data = bit_img.data;
	memset(data, 0x0, iHeight*iWidth);

	memcpy(data, slowdata_bit + (f_cachebit_count*slow_rate + 150)*iWidth*iHeight, iWidth*iHeight);

	resize(bit_img, bit_img, Size(pl_rect.Width(), pl_rect.Height()));
	flip(bit_img, bit_img, 0);
	imshow("PL", bit_img);

}

void CMFCApplication1Dlg::save_raw_data()
{
	BYTE *buffer = new BYTE[FRAME_CUSUM_CNT * 250 * 50];
	int count = 0;
	msg("Saving %d data\n", m_package_count);
	while (m_package_count > 0) {
		WaitForSingleObject(m_rawEvent, INFINITE);
		WaitForSingleObject(m_rawMutex, INFINITE);
		memcpy(buffer, save_data_buffer, FRAME_CUSUM_CNT * 250 * 50);
		ReleaseMutex(m_rawMutex);

		SYSTEMTIME tm;
		CString str;
		CFile file;
		::GetLocalTime(&tm);
		str.Format(".//temdata//%d.dat", count);
		if (file.Open(str, CFile::modeCreate | CFile::modeWrite, NULL))
		{
			file.Write(buffer, FRAME_CUSUM_CNT * 250 * 50);
			file.Close();
			msg("写入文件%s\r\n", str);
			count++;
		}
		else
		{
			msg("Can't open the file to save\n");
		}

		m_package_count--;

	}

	delete[] buffer;
	m_Save_Package = FALSE;
	m_package_count = 0;
	msg("已保存%d个数据成功\n", count);

}

void CMFCApplication1Dlg::DataProc(BOOL bOrgImg, BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight)
{
	param_buffer.windows = this;
	param_buffer.iHeight = iHeight;
	param_buffer.iWidth = iWidth;
	param_buffer.pInData = pInData;
	param_buffer.pOutBuffer = pOutBuffer;
	param_buffer.uDataSize = uDataSize;


	SetEvent(m_Event);

	if (!m_rawproc) {
		m_rawproc = TRUE;

		m_hThread_self = (HANDLE)_beginthreadex(NULL, 0, &show_self, this, 0, 0);
	}

	if (!m_slow_proc) {
		m_slow_proc = TRUE;
		m_hThread_slow_data = (HANDLE)_beginthreadex(NULL, 0, &slow_Thread1, this, 0, 0);

	}

	if (m_Save_Package) {
		if (m_hThread_save == INVALID_HANDLE_VALUE) {
			m_hThread_save = (HANDLE)_beginthreadex(NULL, 0, &save_Thread, this, 0, 0);

		}
	}
	else {
		m_hThread_save = INVALID_HANDLE_VALUE;
	}


}



void CMFCApplication1Dlg::load_and_proc()
{
	UpdateData(TRUE);
	// 保存记录变量，以免中途点击影响到
	BOOL F_video = f_save_slow_video;
	BOOL F_video_bit = f_save_slow_video_bit;
	BOOL F_img = f_save_slow_img;
	BOOL F_img_bit = f_save_slow_img_bit;

	int height = 250;
	int width = 400;
	Mat img(Size(400, 250), CV_8UC1);
	Mat img_bit(Size(400, 250), CV_8UC1);


	uchar *gray = img.data;
	uchar *bit_img = img_bit.data;


	VideoWriter writer;
	VideoWriter writer_bit;
	int codec = CV_FOURCC('F', 'L', 'V', '1');

	double fps = 50.0;                          // framerate of the created video stream
	string video_filename = "./slow_show.avi";             // name of the output video file
	string video_filename_bit = "./slow_show_bit.avi";     // name of the output video file

	writer.open(video_filename, codec, fps, img.size(), 0);

	if (!writer.isOpened()) {
		MessageBox("Can't open the video files to write");

	}

	writer_bit.open(video_filename_bit, codec, fps, img_bit.size(), 0);

	if (!writer_bit.isOpened()) {
		MessageBox("Can't open the bit video files to write");
	}

	//msg("The value of codecc is %d\n", codec);

	vector<string> files;

	char * filePath = ".//temdata";
	char * distAll = "AllFiles.txt";
	getFiles(filePath, files);
	ofstream ofn(distAll);
	int file_count = files.size();
	msg("Files to proc: %d\n", file_count);


	ofn << file_count << endl;
	for (int i = 0; i<file_count; i++)
	{
		ofn << files[i] << endl;
	}
	ofn.close();

	long length = FRAME_CUSUM_CNT * 250 * 50;


	uchar* pulse = new uchar[length];//length为像素个数/8
	uint* interval = new uint[250 * 400];
	uint* label = new uint[250 * 400];


	uchar q1 = 1;
	uchar q2 = 2;
	uchar q3 = 4;
	uchar q4 = 8;
	uchar q5 = 16;
	uchar q6 = 32;
	uchar q7 = 64;
	uchar q8 = 128;

	for (int jj = 0; jj < 250 * 400; jj++)
	{
		label[jj] = 0x00;
	}

	uint itemp = 0;
	uint iframe = 0;
	uint frame;
	uint framecount;
	uint irow = 0;
	uint itime = 0;
	CString filename;

	framecount = 0;
	char tmp[500];


	for (int isize = 0; isize < file_count; isize++)
	{
		filename.Format(".//temdata//%d.dat", isize);
		ifstream file(filename, ios::in | ios::binary);
		file.seekg(0, ios::end);
		length = file.tellg();
		file.seekg(0, ios::beg);
		file.read((char*)pulse, length);
		file.close();

		sprintf(tmp, "Processing %d\\%d \n", isize + 1, file_count);
		msg(tmp);

		for (iframe = 0; iframe< FRAME_CUSUM_CNT; iframe++)
		{
			framecount++;

			memset(bit_img, 0x0, width*height);
			frame = isize * FRAME_CUSUM_CNT + iframe;

			for (irow = 0; irow < 250; irow++)
			{
				for (itime = 0; itime < 50; itime++)//每行52个字节
				{

					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q1)
					{
						interval[8 * 50 * irow + 8 * itime] = frame - label[8 * 50 * irow + 8 * itime];
						label[8 * 50 * irow + 8 * itime] = frame;
						bit_img[8 * 50 * irow + 8 * itime] = 0xff;
					}
					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q2)
					{
						interval[8 * 50 * irow + 8 * itime + 1] = frame - label[8 * 50 * irow + 8 * itime + 1];
						label[8 * 50 * irow + 8 * itime + 1] = frame;
						bit_img[8 * 50 * irow + 8 * itime + 1] = 0xff;
					}
					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q3)
					{
						interval[8 * 50 * irow + 8 * itime + 2] = frame - label[8 * 50 * irow + 8 * itime + 2];
						label[8 * 50 * irow + 8 * itime + 2] = frame;
						bit_img[8 * 50 * irow + 8 * itime + 2] = 0xff;
					}
					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q4)
					{
						interval[8 * 50 * irow + 8 * itime + 3] = frame - label[8 * 50 * irow + 8 * itime + 3];
						label[8 * 50 * irow + 8 * itime + 3] = frame;
						bit_img[8 * 50 * irow + 8 * itime + 3] = 0xff;
					}
					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q5)
					{
						interval[8 * 50 * irow + 8 * itime + 4] = frame - label[8 * 50 * irow + 8 * itime + 4];
						label[8 * 50 * irow + 8 * itime + 4] = frame;
						bit_img[8 * 50 * irow + 8 * itime + 4] = 0xff;
					}
					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q6)
					{
						interval[8 * 50 * irow + 8 * itime + 5] = frame - label[8 * 50 * irow + 8 * itime + 5];
						label[8 * 50 * irow + 8 * itime + 5] = frame;
						bit_img[8 * 50 * irow + 8 * itime + 5] = 0xff;
					}
					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q7)
					{
						interval[8 * 50 * irow + 8 * itime + 6] = frame - label[8 * 50 * irow + 8 * itime + 6];
						label[8 * 50 * irow + 8 * itime + 6] = frame;
						bit_img[8 * 50 * irow + 8 * itime + 6] = 0xff;
					}
					if (pulse[iframe * 50 * 250 + irow * 50 + itime] & q8)
					{
						interval[8 * 50 * irow + 8 * itime + 7] = frame - label[8 * 50 * irow + 8 * itime + 7];
						label[8 * 50 * irow + 8 * itime + 7] = frame;
						bit_img[8 * 50 * irow + 8 * itime + 7] = 0xff;

					}
				}
			}

			for (int mm = 0; mm < 250 * 400; mm++)
			{
				if (interval[mm] != 0)
				{
					gray[mm] = (1600 / interval[mm]> 255) ? 255 : ((1600 / interval[mm]) & 0xff);

				}
				if (interval[mm] == 0)
				{
					gray[mm] = 0;
				}
			}

			flip(img, img, 0);
			flip(img_bit, img_bit, 0);

			if (F_video) {
				writer.write(img);
			}
			if (F_video_bit) {
				writer_bit.write(img_bit);
			}

			if (F_img) {
				sprintf(tmp, "./pic/%d.jpg", framecount);
				imwrite(tmp, img);
			}
			if (F_img_bit) {
				sprintf(tmp, "./bit/%d.jpg", framecount);
				imwrite(tmp, img_bit);
			}

		}

	}

	MessageBox("数据处理完成\n");

	delete[] pulse;
	delete[] interval;
	delete[] label;

	m_raw2video = FALSE;
}
