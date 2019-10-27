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

extern void msg(LPCSTR lpszFmt, ...);
extern UINT __stdcall get_data_thread(LPVOID param);
extern UINT __stdcall rt_display_thread(LPVOID param);
extern UINT __stdcall slow_data_thread(LPVOID param);
extern UINT __stdcall save_raw_thread(LPVOID param);
extern UINT __stdcall offline_proc_thread(LPVOID param);
extern void getFiles(string path, vector<string>& files);


int CMFCApplication1Dlg::enum_dev()
{
	char *DeviceName[12] = { 0 };
	int DeviceNum;
	int i;

	m_wndDevList.ResetContent();
	EnumerateDevice(DeviceName, 12, &DeviceNum);

	for (i = 0; i<DeviceNum; i++)
	{
		if (DeviceName[i] != NULL)
		{
			msg("Found device:%s\n", DeviceName[i]);
			m_wndDevList.AddString(DeviceName[i]);
			m_wndDevList.SetCurSel(m_wndDevList.GetCount() - 1);
			GlobalFree(DeviceName[i]);
		}
	}
	if (DeviceName[0] == NULL)
		return -1;
	else
		return 0;
}

int CMFCApplication1Dlg::open_dev()
{
	CString str = "";
	DWORD Ver[4];
	int iRet;

	if (!m_bOpen)
	{
		m_wndDevList.GetWindowText(str);
		if (str != "")
		{
			iRet = OpenDevice(str, &m_nDevID);
			if (iRet == DT_ERROR_OK)
			{
				msg("Open device(%s) successful\n", str);
				m_bOpen = TRUE;

				//会变换按钮状态，先暂时不要了
				//GetDlgItem(IDC_BUTTON_OPEN)->SetWindowText("Close");

				if (GetLibVersion(Ver, m_nDevID) == DT_ERROR_OK)
				{
					msg("LIB Version:%d.%d.%d.%d\n", Ver[0], Ver[1], Ver[2], Ver[3]);
				}

				if (GetFwVersion(0, Ver, m_nDevID) == DT_ERROR_OK)
				{
					msg("FW1 Version:%d.%d.%d.%d\n", Ver[0], Ver[1], Ver[2], Ver[3]);
				}

				if (GetFwVersion(1, Ver, m_nDevID) == DT_ERROR_OK)
				{
					msg("FW2 Version:%d.%d.%d.%d\n", Ver[0], Ver[1], Ver[2], Ver[3]);
				}

				if (GetFwVersion(2, Ver, m_nDevID) != DT_ERROR_OK)
				{
					msg("FW3 Version:%d.%d.%d.%d\n", Ver[0], Ver[1], Ver[2], Ver[3]);
				}
				if (GetHardwareVersion(Ver, m_nDevID) != DT_ERROR_OK)
				{
					msg("HW Version:%d.%d.%d.%d\n", Ver[0], Ver[1], Ver[2], Ver[3]);
				}
			}
			else
			{
				msg("OpenDevice(%s) failed with err:%d\n", str, iRet);
			}
		}
	}
	else
	{
		
		if (m_bRunning)
		{
			/* 先关闭测试线程 */
			start_dev();
		}

		iRet = CloseDevice(m_nDevID);
		if (iRet == DT_ERROR_OK)
		{
			msg("CloseDevice OK\n", str, iRet);
			m_bOpen = FALSE;
			// 同上 GetDlgItem(IDC_BUTTON_OPEN)->SetWindowText("Open");
		}
		else
		{
			msg("CloseDevice failed with err:%d\n", iRet);
		}
	}
	return 0; // tmp
}

int CMFCApplication1Dlg::start_dev()
{
	if (m_hThread == INVALID_HANDLE_VALUE)
	{

		UpdateData(TRUE);
		// 启动工作线程
		m_bRunning = TRUE;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &get_data_thread, this, 0, 0);


	}
	else
	{
		m_bRunning = FALSE;
		WaitForSingleObject(m_hThread, INFINITE);

		m_hThread = INVALID_HANDLE_VALUE;

	}
	return 0;
}


void CMFCApplication1Dlg::WorkProc()
{
	int iRet;

	msg("WorkProc Enter\n");
	/* 调试代码，验证电压值正确与否
	msg("The value of clk is: %f\n", m_fMclk);
	msg("The value of m_fAvdd is: %f\n", m_fAvdd);
	msg("The value of m_fDovdd is: %f\n", m_fDovdd);
	msg("The value of m_fDvdd is: %f\n", m_fDvdd);
	msg("The value of m_fAfvcc is: %f\n", m_fAfvcc);
	msg("The value of m_fVpp is: %f\n", m_fVpp);
	*/
	/*
	m_fMclk = 10.0;
	m_fAvdd = 2.8;
	m_fDovdd = 1.8;
	m_fDvdd = 1.8;
	m_fAfvcc = 1.8;
	m_fVpp = 5.0;
	*/
	SensorIniFile SensorIni(m_strIniFile);

	if (SensorIni.LoadIniFile())
	{
		msg("Load sensor ini OK\n");
	}
	else
	{
		msg("Load sensor ini failed\n");
	}


	// 初始化柔性IO
	UCHAR PinDef[26] = { PIN_NC };

	PinDef[0] = PIN_CLK_ADJ_18M;
	PinDef[1] = 0;
	PinDef[2] = 2;
	PinDef[3] = 1;
	PinDef[4] = 3;
	PinDef[5] = 4;
	PinDef[6] = 5;
	PinDef[7] = 6;
	PinDef[8] = 7;
	PinDef[9] = 8;
	PinDef[10] = 9;
	PinDef[11] = 20;
	PinDef[12] = 10;
	PinDef[13] = 11;
	PinDef[14] = 12;
	PinDef[15] = 20;
	PinDef[16] = 20;
	PinDef[17] = 13;
	PinDef[18] = 15;				//PIN_PWDN PIN_SPI_CS 15
	PinDef[19] = 14;
	PinDef[20] = PIN_SPI_SCK;					//19 //PIN_SPI_SCK
	PinDef[21] = PIN_SPI_SDO;					//PIN_SPI_SDO 18
	PinDef[22] = PIN_SPI_CS;
	PinDef[23] = PIN_CLK_ADJ_18M;		//PIN_SPI_SDO
	PinDef[24] = PIN_GPIO3;			//PIN_SPI_SDI/PIN_SPI_SDA //hs
	PinDef[25] = PIN_GPIO4;				//vs

	SetSoftPin(PinDef, m_nDevID);
	EnableSoftPin(TRUE, m_nDevID);
	SetSoftPinPullUp(TRUE, m_nDevID);
	EnableGpio(TRUE, m_nDevID);


	// 电源设置
	// 初始化SENOSR电源
	SENSOR_POWER Power[5];
	int Volt[5];
	int Rise[5];
	BOOL OnOff[5];
	int Current[5];
	CURRENT_RANGE Range[5];
	int SampleSpeed[5];

	Power[0] = POWER_AVDD;
	Volt[0] = (int)(m_fAvdd * 1000); // 2.8V
	Current[0] = 600; // 500mA
	Rise[0] = 100;
	OnOff[0] = TRUE;
	Range[0] = CURRENT_RANGE_MA;
	SampleSpeed[0] = 200;

	Power[1] = POWER_DOVDD;
	Volt[1] = (int)(m_fDovdd * 1000); // 1.8V
	Current[1] = 600; // 500mA
	Rise[1] = 100;
	OnOff[1] = TRUE;
	Range[1] = CURRENT_RANGE_MA;
	SampleSpeed[1] = 200;

	Power[2] = POWER_DVDD;
	Volt[2] = (int)(m_fDvdd * 1000);// 1.8V
	Current[2] = 600;// 600mA
	Rise[2] = 100;
	OnOff[2] = TRUE;
	Range[2] = CURRENT_RANGE_MA;
	SampleSpeed[2] = 200;

	Power[3] = POWER_AFVCC;
	Volt[3] = (int)(m_fAfvcc * 1000); // 2.8V
	Current[3] = 600; // 600mA
	Rise[3] = 100;
	OnOff[3] = TRUE;
	Range[3] = CURRENT_RANGE_MA;
	SampleSpeed[3] = 200;

	Power[4] = POWER_VPP;
	Volt[4] = (int)(m_fVpp * 1000); // 2.8V
	Current[4] = 600; // 600mA
	Rise[4] = 100;
	OnOff[4] = TRUE;
	Range[4] = CURRENT_RANGE_MA;
	SampleSpeed[4] = 200;


	// 设置电压斜率
	if (PmuSetRise(Power, Rise, 5, m_nDevID) != DT_ERROR_OK)
	{
		msg("设置电压斜率失败!\r\n");
		//return 0;
	}

	if (PmuSetSampleSpeed(Power, SampleSpeed, 5, m_nDevID) != DT_ERROR_OK)
	{
		msg("设置电流采样速度失败!\r\n");
		//return 0;
	}

	// 先设置电压值为0
	Volt[0] = 0;
	Volt[1] = 0;
	Volt[2] = 0;
	Volt[3] = 0;
	Volt[4] = 0;

	// 设置电压值
	if (PmuSetVoltage(Power, Volt, 5, m_nDevID) != DT_ERROR_OK)
	{
		msg("初始化电源失败!\r\n");
		//return DT_ERROR_FAILED;
	}

	OnOff[0] = TRUE;
	OnOff[1] = TRUE;
	OnOff[2] = TRUE;
	OnOff[3] = TRUE;
	OnOff[4] = TRUE;

	if (PmuSetOnOff(Power, OnOff, 5, m_nDevID) != DT_ERROR_OK)
	{
		msg("开启电源失败!\r\n");
		//return DT_ERROR_FAILED;
	}

	Volt[0] = (int)(m_fAvdd * 1000); // 2.8V
	Volt[1] = (int)(m_fDovdd * 1000); // 1.8V
	Volt[2] = (int)(m_fDvdd * 1000);// 1.8V
	Volt[3] = (int)(m_fAfvcc * 1000); // 2.8V
	Volt[4] = (int)(m_fVpp * 1000); // 2.8V

									// 设置电压值
	if (PmuSetVoltage(Power, Volt, 5, m_nDevID) != DT_ERROR_OK)
	{
		msg("初始化电源失败!\r\n");
		//return DT_ERROR_FAILED;
	}

	if (PmuSetOcpCurrentLimit(Power, Current, 5, m_nDevID) != DT_ERROR_OK)
	{
		msg("初始化电源限流失败!\r\n");
	}
	// 设置量程
	if (PmuSetCurrentRange(Power, Range, 5, m_nDevID) != DT_ERROR_OK)
	{
		msg("设置量程失败！\r\n");
	}

	iRet = SetSensorClock(TRUE, (USHORT)(m_fMclk * 10), m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("Set MCLK failed with err:%d\n", iRet);
	}
	Sleep(50);


	// 设置reset pin, pwdn pin
	BYTE Pwdn2 = 0;
	BYTE Pwdn1 = 0;
	BYTE Reset = 0;

	SensorEnable(SensorIni.m_tab.pin^RESET_H, TRUE, m_nDevID);
	Sleep(50);
	SensorEnable(SensorIni.m_tab.pin, TRUE, m_nDevID);
	Sleep(50);

	Pwdn2 = SensorIni.m_tab.pin & PWDN_H ? PWDN2_L : PWDN2_H;   //pwdn2 neg to pwdn1
	Pwdn1 = SensorIni.m_tab.pin & PWDN_H ? PWDN_H : PWDN_L;     //pwdn1
	Reset = SensorIni.m_tab.pin & RESET_H ? RESET_H : RESET_L;  //reset

	SensorIni.m_tab.pin = Pwdn2 | Pwdn1 | Reset;
	SensorEnable(SensorIni.m_tab.pin, 1, m_nDevID); //reset


													// 图像采集过程
	BOOL bOrgImage = m_bOriginalImage; // 记录变量，暂不支持在采集过程中改变
	int iWidth = SensorIni.m_tab.width * 8; // 记录实际像素宽高信息
	int iHeight = SensorIni.m_tab.height;


	m_uFrameCusum = FRAME_CUSUM_CNT;

	// 设置40k fps sensor的专属配置，采集原始图像或积分图像
	// 采集这么高帧率的图像不能按常规方式采集， 修改宽高信息可以达到等效的数据量
	if (bOrgImage)
	{
		// 专用的配置命令，设置采集原始图像，每m_uFrameCusum帧一簇
		iRet = ExCtrl(1000, m_uFrameCusum, NULL, NULL, NULL, NULL, m_nDevID);

		// 使采集数据量匹配
		SensorIni.m_tab.width *= m_uFrameCusum;
	}
	else
	{
		// 专用的配置命令，设置采集积分图像，每m_uFrameCusum帧一簇
		iRet = ExCtrl(1000, m_uFrameCusum | 0x80000000, NULL, NULL, NULL, NULL, m_nDevID);

		// 使采集数据量匹配
		SensorIni.m_tab.width *= 4 * 7 * (m_uFrameCusum / 100);
	}

	if (iRet != DT_ERROR_OK)
	{
		msg("ExCtrl failed with err:%d\n", iRet);
	}

	ULONG uSize = SensorIni.m_tab.width * SensorIni.m_tab.height;
	BYTE *pBuffer = new BYTE[uSize];
	if (!pBuffer)
		msg("Can alloc  buffer\n");
	BYTE *pOutBuffer = new BYTE[iWidth*iHeight * 2]; // 如果是积分数据，每个像素为16位的整数
	ULONG uDataSize;

	iRet = InitSensor(SensorIni.m_tab.SlaveID, SensorIni.m_tab.ParaList, SensorIni.m_tab.ParaListSize, SensorIni.m_tab.mode, m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("InitSensor failed with err:%d\n", iRet);
	}

	iRet = InitDevice(m_wndVideo.GetSafeHwnd(), SensorIni.m_tab.width, SensorIni.m_tab.height, SensorIni.m_tab.port, SensorIni.m_tab.type, CHANNEL_A, NULL, m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("InitDevice failed with err:%d\n", iRet);
	}

	iRet = OpenVideo(uSize, m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("OpenVideo failed with err:%d\n", iRet);
	}
	m_uImageBytes = 0;
	m_uFrameCnt = 0;
	m_uFrameCntLast = 0;



	while (m_bRunning)
	{

		//start = getTickCount();
		iRet = GrabFrameEx(pBuffer, uSize, &uDataSize, &m_frameInfo, m_nDevID);
		/*
		duration = (getTickCount() - start) /
		getTickFrequency();
		msg( "The timeout is :%.8f\n", duration);
		*/
		if (iRet == DT_ERROR_OK)
		{


			m_uFrameCnt++;
			m_uImageBytes = uDataSize;

			WaitForSingleObject(Mutex_rt, INFINITE);
			memcpy(raw_data, pBuffer, FRAME_CUSUM_CNT * 250 * 50);
			ReleaseMutex(Mutex_rt);

			// 是否保原始数据
			if (m_package_count > 0) {
				WaitForSingleObject(Mutex_save, INFINITE);
				memcpy(save_data_buffer, pBuffer, FRAME_CUSUM_CNT * 250 * 50);
				ReleaseMutex(Mutex_save);
				//唤醒沉睡的保存线程
				SetEvent(Event_save);

			}
			
			//是否到达下个周期的慢速展示
			if (m_temp_count > 0) {
				memcpy(temp_data_buffer+(5-m_temp_count)*FRAME_CUSUM_CNT*250*50
					, pBuffer, FRAME_CUSUM_CNT * 250 * 50);
				m_temp_count--;
				if (m_temp_count == 0)
				{
					SetEvent(Event_slow);
					//msg("测试间隔时间\n");
				}
					
			}
			if (!m_bRunning)
				break;


			DataProc(bOrgImage, pBuffer, uDataSize, pOutBuffer, iWidth, iHeight);
		}

	}

	iRet = CloseVideo(m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("Close failed with err:%d\n", iRet);
	}

	// 关闭sensor
	SensorEnable(SensorIni.m_tab.pin, FALSE, m_nDevID);

	// 关闭时钟
	SetSensorClock(FALSE, (USHORT)(m_fMclk * 10), m_nDevID);

	// 关闭电源
	Power[0] = POWER_AVDD;
	Volt[0] = 0;
	OnOff[0] = FALSE;

	Power[1] = POWER_DOVDD;
	Volt[1] = 0;
	OnOff[1] = FALSE;

	Power[2] = POWER_DVDD;
	Volt[2] = 0;
	OnOff[2] = FALSE;

	Power[3] = POWER_AFVCC;
	Volt[3] = 0;
	OnOff[3] = FALSE;

	Power[4] = POWER_VPP;
	Volt[4] = 0;
	OnOff[4] = FALSE;

	PmuSetVoltage(Power, Volt, 5, m_nDevID);
	PmuSetOnOff(Power, OnOff, 5, m_nDevID);

	// 关闭柔性接口
	SetSoftPinPullUp(FALSE, m_nDevID);
	EnableSoftPin(FALSE, m_nDevID);
	EnableGpio(FALSE, m_nDevID);

	if (pBuffer != NULL)
		delete[] pBuffer;

	if (pOutBuffer != NULL)
		delete[] pOutBuffer;

	msg("WorkProc Exit\n");
}



void CMFCApplication1Dlg::SetPower()
{
	UpdateData(TRUE);

	//设置电压，电流
	SENSOR_POWER Power[10];
	int Volt[10];
	int Current[10];
	BOOL OnOff[10];
	CURRENT_RANGE range[5];

	Power[0] = POWER_AVDD;
	Volt[0] = (int)(m_fAvdd * 1000); // 2.8V
	Current[0] = 300; // 300mA
	OnOff[0] = TRUE;
	range[0] = CURRENT_RANGE_MA;

	Power[1] = POWER_DOVDD;
	Volt[1] = (int)(m_fDovdd * 1000); // 1.8V
	Current[1] = 300; // 300mA
	OnOff[1] = TRUE;
	range[1] = CURRENT_RANGE_MA;

	Power[2] = POWER_DVDD;
	Volt[2] = (int)(m_fDvdd * 1000);// 1.2V
	Current[2] = 300;// 300mA
	OnOff[2] = TRUE;
	range[2] = CURRENT_RANGE_MA;

	Power[3] = POWER_AFVCC;
	Volt[3] = (int)(m_fAfvcc * 1000); // 2.8V
	Current[3] = 300; // 300mA
	OnOff[3] = TRUE;
	range[3] = CURRENT_RANGE_MA;

	Power[4] = POWER_VPP;
	Volt[4] = (int)(m_fVpp * 1000);
	Current[4] = 300; // 300mA
	OnOff[4] = TRUE;
	range[4] = CURRENT_RANGE_MA;

	//设置5路电压值
	int iRet = PmuSetVoltage(Power, Volt, 5, m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("Set Voltage failed with err:%d\n", iRet);
	}
	else
	{
		msg("成功设置电压值\n");
	}
}

void CMFCApplication1Dlg::SetFreq()
{
	UpdateData(TRUE);
	int iRet = SetSensorClock(TRUE, (USHORT)(m_fMclk * 10), m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("Set MCLK failed with err:%d\n", iRet);
	}
	else
	{
		msg("成功设置工作频率\n");
	}
}