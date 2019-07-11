
// MFCApplication1Dlg.cpp : 实现文件
#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include<tchar.h>
#include <opencv2/opencv.hpp>


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
class cparamlist {
public:
	CMFCApplication1Dlg *windows;
	BYTE *pInData;
	ULONG uDataSize;
	BYTE *pOutBuffer;
	int iWidth;
	int iHeight;
};

cparamlist param_buffer;

#pragma comment (lib,".\\DTCCM2_SDK\\dtccm2.lib")

UINT __stdcall RemoteDebugThread(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	pDlg->WorkProc();
	return 0;
}


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


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

														// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 对话框



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
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &RemoteDebugThread, this, 0, 0);

	}
	else
	{
		m_bRunning = FALSE;
		WaitForSingleObject(m_hThread, INFINITE);
	
		m_hThread = INVALID_HANDLE_VALUE;
		
	}
	return 0;
}





CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
	, m_strIniFile(_T(".\\40kfps.ini"))
	, m_bOpen(FALSE)
	, m_nDevID(0)
	, m_bRunning(FALSE)
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_hThread_save(INVALID_HANDLE_VALUE)
	, m_fAvdd(2.8f)
	, m_fDovdd(1.8f)
	, m_fDvdd(1.8f)
	, m_fAfvcc(1.8f)
	, m_fVpp(5.0f)
	, m_fMclk(10.0f)
	, m_fSavetime(200.0f)
	, m_uImageBytes(0)
	, m_fFrameRate(0.0f)
	, m_uFrameCntLast(0)
	, m_bOriginalImage(TRUE)
	, m_Save_Package(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	slowdata = new BYTE[FRAME_CUSUM_CNT * 250 * 400];
	slowdata_bit = new BYTE[FRAME_CUSUM_CNT * 250 * 400];

	raw_data = new BYTE[FRAME_CUSUM_CNT * 250 * 80]; // 分配原始数据包的缓冲区
	save_data_buffer = new BYTE[FRAME_CUSUM_CNT * 250 * 80];

}
CMFCApplication1Dlg::~CMFCApplication1Dlg()
{
	
	if (m_bRunning) {


		m_bRunning = FALSE;

		WaitForSingleObject(m_hThread, INFINITE);
		m_rawproc = FALSE;
		m_bitproc = FALSE;
		WaitForSingleObject(m_hThread_slow, INFINITE);
		WaitForSingleObject(m_hThread_self, INFINITE);

		int iRet = CloseDevice(m_nDevID);
		if (iRet != DT_ERROR_OK)
		{
			MessageBox("ERROR!!! Can't close the ");
		}
	}
	
	delete[] slowdata;
	delete[] slowdata_bit;
	delete[] raw_data;
	delete[] save_data_buffer;
}


void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_MSG2, m_editMsg);
	DDX_Control(pDX, RT_Display, m_wndVideo);
	//DDX_Control(pDX, Slow_Display, m_slow);
	DDX_Control(pDX, RT_FPS, m_fps);
	DDX_Control(pDX, Rawdata, m_rawdata);
	DDX_Control(pDX, SLOWIMG, m_slowimg);
	DDX_Control(pDX, CAM_CHOOSE, m_wndDevList);

	DDX_Text(pDX, SAVEMS, m_fSavetime);

	DDX_Check(pDX, save_slow_video, f_save_slow_video);
	DDX_Check(pDX, save_slow_video_bit, f_save_slow_video_bit);
	DDX_Check(pDX, save_slow_img, f_save_slow_img);
	DDX_Check(pDX, save_slow_img_bit, f_save_slow_img_bit);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_MSG, OnMsg)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(Find_cam, &CMFCApplication1Dlg::Found_Cam)
	ON_BN_CLICKED(CLOSE_CAM, &CMFCApplication1Dlg::OnBnClickedCam)
	ON_BN_CLICKED(SET_SAVETIME, &CMFCApplication1Dlg::SetSaveTime)
	ON_BN_CLICKED(Offline_Data2, &CMFCApplication1Dlg::raw2video)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	CWnd  *pWnd1 = GetDlgItem(RT_Display);//CWnd是MFC窗口类的基类,提供了微软基础类库中所有窗口类的基本功能。
	pWnd1->GetClientRect(&rt_rect);//GetClientRect为获得控件相自身的坐标大小
	namedWindow("RT", WINDOW_AUTOSIZE);//设置窗口名
	HWND hWndl = (HWND)cvGetWindowHandle("RT");//hWnd 表示窗口句柄,获取窗口句柄
	HWND hParent1 = ::GetParent(hWndl);//GetParent函数一个指定子窗口的父窗口句柄
	::SetParent(hWndl, GetDlgItem(RT_Display)->m_hWnd);
	::ShowWindow(hParent1, SW_HIDE);//ShowWindow指定窗口中显


	CWnd  *pWnd2 = GetDlgItem(Pulse_Display);//CWnd是MFC窗口类的基类,提供了微软基础类库中所有窗口类的基本功能。
	pWnd2->GetClientRect(&pl_rect);//GetClientRect为获得控件相自身的坐标大小
	namedWindow("PL", WINDOW_AUTOSIZE);//设置窗口名
	HWND hWnd2 = (HWND)cvGetWindowHandle("PL");//hWnd 表示窗口句柄,获取窗口句柄
	HWND hParent2 = ::GetParent(hWnd2);//GetParent函数一个指定子窗口的父窗口句柄
	::SetParent(hWnd2, GetDlgItem(Pulse_Display)->m_hWnd);
	::ShowWindow(hParent2, SW_HIDE);//ShowWindow指定窗口中显

	CWnd  *pWnd3 = GetDlgItem(Slow_Display);//CWnd是MFC窗口类的基类,提供了微软基础类库中所有窗口类的基本功能。
	pWnd3->GetClientRect(&sl_rect);//GetClientRect为获得控件相自身的坐标大小
	namedWindow("SL", WINDOW_AUTOSIZE);//设置窗口名
	HWND hWnd3 = (HWND)cvGetWindowHandle("SL");//hWnd 表示窗口句柄,获取窗口句柄
	HWND hParent3 = ::GetParent(hWnd3);//GetParent函数一个指定子窗口的父窗口句柄
	::SetParent(hWnd3, GetDlgItem(Slow_Display)->m_hWnd);
	::ShowWindow(hParent3, SW_HIDE);//ShowWindow指定窗口中显

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MAXIMIZE);

	// TODO: 在此添加额外的初始化代码
	m_Mutex = CreateMutex(NULL, FALSE, NULL);
	m_rawMutex = CreateMutex(NULL, FALSE, NULL);


	// 500毫秒的定时器，用作状态刷新
	SetTimer(0, 500, NULL);
	// 用于统计帧率
	SetTimer(1, 4000, NULL);
	// 用于判断是否进行离散脉冲慢速展示
	
	int ret;
	memset(&m_frameInfo, 0x00, sizeof(m_frameInfo));
	m_strIniFile = ".\\40kfps.ini";

	ret = enum_dev();
	if (ret) {
		MessageBox("No Camera Found!");
	}
	else
	{
		open_dev();
		start_dev();
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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
			
			WaitForSingleObject(m_Mutex, INFINITE);
			memcpy(raw_data , pBuffer, FRAME_CUSUM_CNT * 250 * 50);
			ReleaseMutex(m_Mutex);

			// 是否保原始数据
			if (m_package_count > 0) {
				WaitForSingleObject(m_rawMutex, INFINITE);
				memcpy(save_data_buffer, pBuffer, FRAME_CUSUM_CNT * 250 * 50);
				ReleaseMutex(m_rawMutex);
				SetEvent(m_rawEvent);

			}
			
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
	msg("已保存%d个数据成功\n",count);

}

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
		memcpy(pulse, raw_data,  FRAME_CUSUM_CNT * 250 * 50);
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

	while (m_bitproc) {

		if (f_cachebit) {
			f_cachebit = FALSE;

			memcpy(pulse, raw_data, FRAME_CUSUM_CNT * 250 * 50);

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
				memcpy(slowdata + iframe * 400 * 250, gray, 400 * 250);
				memcpy(slowdata_bit + iframe * 400 * 250, bit_img, 400 * 250);
			}
		}

		Sleep(10);
		Display_slow(iWidth, iHeight);
	}

	delete[] pulse;
	delete [] interval;
	delete [] label ;
	delete [] gray_first;
	delete [] gray;
	delete [] bit_img;
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

	if (!m_bitproc) {
		m_bitproc = TRUE;
		m_hThread_slow = (HANDLE)_beginthreadex(NULL, 0, &slow_Thread1, this, 0, 0);

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





LRESULT CMFCApplication1Dlg::OnMsg(WPARAM wP, LPARAM lP)
{
	static int nLineCount = 0;

	char* lpszMsg = (LPSTR)wP;

	if (m_editMsg.GetSafeHwnd() != NULL)
	{
		if (nLineCount >= 10000)
		{
			nLineCount = 0;
			m_editMsg.Clear();
		}

		int len = m_editMsg.GetWindowTextLength();
		if (len >= 0)
		{
			if (len > 10000)
			{
				m_editMsg.Clear();
				m_editMsg.SetWindowText(lpszMsg);
			}
			else
			{
				m_editMsg.SetSel(len, len);
				m_editMsg.ReplaceSel(lpszMsg);
			}
		}
	}
	if (wP != NULL)
	{
		free(lpszMsg);
	}
	return 0;
}

void CMFCApplication1Dlg::Found_Cam()
{
	if (m_bRunning) {
		MessageBox("相机已经在运行！！！！");
	} 
	else
	{
		enum_dev();
		open_dev();
		start_dev();
	}
	
	if (!m_bRunning) {
		OnBnClickedCam();
		msg("Can't Found the Camera!");
		
	}
	
	// TODO: 在此添加控件通知处理程序代码
}


void CMFCApplication1Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CString str;
		CString str_raw;
		CString str_slow;

		str.Format("FrameCount: %u, Rate: %.3ffps\n",
			m_uFrameCnt, m_fFrameRate);

		str_raw.Format("前 %d 秒的原始脉冲图像放慢 %d 倍显示\n", 
			 f_cachebit_count/ 2, (int) m_fFrameRate * 400 / slow_rate);
			
		str_slow.Format("前 %d 秒脉冲间隔恢复图像放慢 %d 倍显示\n",
			 f_cachebit_count / 2, (int) m_fFrameRate * 400 / slow_rate);

		m_fps.SetWindowText(str);
		m_rawdata.SetWindowText(str_raw);
		m_slowimg.SetWindowText(str_slow);


		f_cachebit_count++;
		if (f_cachebit_count % 10 == 0) {
			f_cachebit = TRUE;
			f_cachebit_count = 0;
		}
			
	}
	else if (nIDEvent == 1)
	{
		m_fFrameRate = (float)(m_uFrameCnt - m_uFrameCntLast) / 4;
		m_uFrameCntLast = m_uFrameCnt;
	}
	
	CDialogEx::OnTimer(nIDEvent);
}

void CMFCApplication1Dlg::OnBnClickedCam()
{
	if (!m_bRunning) {
		MessageBox("相机未在工作，无需关闭");
	}
	else {
	
		m_bRunning = FALSE;

		WaitForSingleObject(m_hThread, INFINITE);
		m_rawproc = FALSE;
		m_bitproc = FALSE;
		WaitForSingleObject(m_hThread_slow, INFINITE);
		WaitForSingleObject(m_hThread_self, INFINITE);

		m_hThread = INVALID_HANDLE_VALUE;
		m_hThread_slow = INVALID_HANDLE_VALUE;
		m_hThread_self = INVALID_HANDLE_VALUE;

		int iRet = CloseDevice(m_nDevID);
		if (iRet == DT_ERROR_OK)
		{
			msg("CloseDevice OK\n");
			m_bOpen = FALSE;

		}
		else
		{
			msg("CloseDevice failed with err:%d\n", iRet);
		}
	}
	
}


void CMFCApplication1Dlg::SetSaveTime()
{
	
	UpdateData(TRUE);

	int savems = ((int)m_fSavetime) - (((int)m_fSavetime) % 20);
	m_package_count = savems / 20 + 1;
	msg("Setting %d ms data\n", savems);

	m_Save_Package = TRUE;
	// TODO: 在此添加控件通知处理程序代码
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

		sprintf(tmp, "Processing %d\\%d \n", isize+1 ,file_count);
		msg(tmp);

		for (iframe  = 0; iframe< FRAME_CUSUM_CNT; iframe++)
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



void CMFCApplication1Dlg::raw2video()
{
	if (m_raw2video) {
		MessageBox("The offline thread is running");
	}
	else {
		m_raw2video = TRUE;
		//if (m_hThread_raw2video == INVALID_HANDLE_VALUE)
			m_hThread_raw2video = (HANDLE)_beginthreadex(NULL, 0, &offline_data_proc, this, 0, 0);

	}


	// TODO: 在此添加控件通知处理程序代码
}

