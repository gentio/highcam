
// MFCApplication1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include<tchar.h>
#include <opencv2/opencv.hpp>
//#include <opencv.hpp>

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



#pragma comment (lib,".\\DTCCM2_SDK\\dtccm2.lib")
UINT cunum;
UINT time_sv, fre, cu_e;

BYTE *slowdata = new BYTE[FRAME_CUSUM_CNT * 250 * 400];
BYTE *slowdata_bit = new BYTE[FRAME_CUSUM_CNT * 250 * 400];


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

UINT __stdcall Thread1(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	for (int i = 0; i < 10000; i++)
		msg("Thread from 1: %d\n", i);
	return 0;
}

UINT __stdcall Thread2(LPVOID param)
{
	CMFCApplication1Dlg *pDlg = (CMFCApplication1Dlg*)param;
	for (int i = 0; i < 10000; i++)
		msg("Thread from 2: %d\n", i);
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

	//m_wndDevList.ResetContent();

	EnumerateDevice(DeviceName, 12, &DeviceNum);

	for (i = 0; i<DeviceNum; i++)
	{
		if (DeviceName[i] != NULL)
		{
			msg("Found device:%s\n", DeviceName[i]);
			//m_wndDevList.AddString(DeviceName[i]);
			//m_wndDevList.SetCurSel(m_wndDevList.GetCount() - 1);
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
	CString str = "U40K_2";
	DWORD Ver[4];
	int iRet;

	if (!m_bOpen)
	{
		//m_wndDevList.GetWindowText(str);
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
		cunum = 0;
		if (m_bOriginalImage)
		{

			fre = 10;
			time_sv = 200;
			cu_e = fre*time_sv * 2 / 400;
			//fre = GetDlgItemInt(IDC_EDITMCLK);
			//time_sv = GetDlgItemInt(IDC_EDITSAVETIME);
			//cu_e = fre*time_sv * 2 / 400;
		}
		fre = 10;
		time_sv = 200;
		cu_e = fre*time_sv * 2 / 400;
		UpdateData(TRUE);
		// 启动工作线程
		m_bRunning = TRUE;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &RemoteDebugThread, this, 0, 0);
	//	m_hThread1 = (HANDLE)_beginthreadex(NULL, 0, &Thread1, this, 0, 0);
	//	m_hThread2 = (HANDLE)_beginthreadex(NULL, 0, &Thread2, this, 0, 0);
		if (m_hThread != INVALID_HANDLE_VALUE)
		{
			//GetDlgItem(IDC_BUTTON_START)->SetWindowText("stop");
			msg("ENTER THREAD\n");
		}
	}
	else
	{
		//保存数据
		if (!m_bOriginalImage)
		{
			CFile file;

			// 估计这里是保存了10个数据包，pic.dat大概有48m
			file.Open("pic.dat", CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite, NULL);
			//file.Write(STOREAGE, cu_e * 400 * 250);
			file.Close();
		}
		// 让工作线程退
		m_bSave = FALSE;
		m_bRunning = FALSE;
		WaitForSingleObject(m_hThread, INFINITE);
	//	WaitForSingleObject(m_hThread1, INFINITE);
	//	WaitForSingleObject(m_hThread2, INFINITE);
		m_hThread = INVALID_HANDLE_VALUE;
		//GetDlgItem(IDC_BUTTON_START)->SetWindowText("start");
	}
	return 0;
}

void CMFCApplication1Dlg::DrawImage(BYTE *pRgb, int iWidth, int iHeight)
{
	BYTE *pRGB = pRgb;
	BITMAPINFOHEADER bmpInfoHeader;

	memset(&bmpInfoHeader, 0x00, sizeof(BITMAPINFOHEADER));
	bmpInfoHeader.biWidth = 4384;
	bmpInfoHeader.biHeight = 3288;
	bmpInfoHeader.biSize = sizeof(bmpInfoHeader);
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biSizeImage = 0;

	HWND hWnd = GetDlgItem(RT_Display)->GetSafeHwnd();

	HDRAWDIB bmpDC;
	HDC hdc = ::GetDC(hWnd);
	bmpDC = DrawDibOpen();

	CRect rect;
	int iDispWidth = 0;
	int iDispHeight = 0;

	//CameraGetImageSize(m_iWorkCam, &bROI, &iHOff, &iVOff, &iWidth, &iHeight);
	bmpInfoHeader.biSize = sizeof(bmpInfoHeader);
	bmpInfoHeader.biWidth = iWidth;
	bmpInfoHeader.biHeight = iHeight;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biSizeImage = 0;
	::GetWindowRect(hWnd, &rect);
	iDispWidth = rect.Width();
	iDispHeight = rect.Height();

	DrawDibDraw(bmpDC, hdc,
		0, 0,
		iDispWidth, iDispHeight,
		&bmpInfoHeader,
		pRGB,
		0, 0, -1, -1,
		0);

	if (bmpDC != NULL)
	{
		DrawDibClose(bmpDC);
		bmpDC = NULL;
	}
	::ReleaseDC(hWnd, hdc);
}

void CMFCApplication1Dlg::DrawImage_slow(BYTE *pRgb, int iWidth, int iHeight)
{
	BYTE *pRGB = pRgb;
	BITMAPINFOHEADER bmpInfoHeader;

	memset(&bmpInfoHeader, 0x00, sizeof(BITMAPINFOHEADER));
	bmpInfoHeader.biWidth = 4384;
	bmpInfoHeader.biHeight = 3288;
	bmpInfoHeader.biSize = sizeof(bmpInfoHeader);
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biSizeImage = 0;

	HWND hWnd = GetDlgItem(Slow_Display)->GetSafeHwnd();

	HDRAWDIB bmpDC;
	HDC hdc = ::GetDC(hWnd);
	bmpDC = DrawDibOpen();

	CRect rect;
	int iDispWidth = 0;
	int iDispHeight = 0;

	//CameraGetImageSize(m_iWorkCam, &bROI, &iHOff, &iVOff, &iWidth, &iHeight);
	bmpInfoHeader.biSize = sizeof(bmpInfoHeader);
	bmpInfoHeader.biWidth = iWidth;
	bmpInfoHeader.biHeight = iHeight;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biSizeImage = 0;
	::GetWindowRect(hWnd, &rect);
	iDispWidth = rect.Width();
	iDispHeight = rect.Height();

	DrawDibDraw(bmpDC, hdc,
		0, 0,
		iDispWidth, iDispHeight,
		&bmpInfoHeader,
		pRGB,
		0, 0, -1, -1,
		0);

	if (bmpDC != NULL)
	{
		DrawDibClose(bmpDC);
		bmpDC = NULL;
	}
	::ReleaseDC(hWnd, hdc);
}



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
	, m_strIniFile(_T(".\\40kfps.ini"))
	, m_bOpen(FALSE)
	, m_nDevID(0)
	, m_bRunning(FALSE)
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_fAvdd(2.8f)
	, m_fDovdd(1.8f)
	, m_fDvdd(1.2f)
	, m_fAfvcc(2.8f)
	, m_fVpp(0.0f)
	, m_fMclk(12.0f)
	, m_fSavetime(2000.0f)
	, m_uFrameCnt(0)
	, m_uImageBytes(0)
	, m_fFrameRate(0.0f)
	, m_uFrameCntLast(0)
	, m_bOriginalImage(TRUE)
	, m_Video(FALSE)
	, m_bSave(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);



	
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_MSG2, m_editMsg);
	DDX_Control(pDX, RT_Display, m_wndVideo);
	//DDX_Control(pDX, Slow_Display, m_slow);
	DDX_Control(pDX, RT_FPS, m_fps);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_MSG, OnMsg)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(Find_cam, &CMFCApplication1Dlg::Found_Cam)
	ON_BN_CLICKED(CLOSE_CAM, &CMFCApplication1Dlg::OnBnClickedCam)
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
	

	f_cachebit = 0;
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
		enum_dev();
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
	m_fMclk = 10.0;
	m_fAvdd = 2.8;
	m_fDovdd = 1.8;
	m_fDvdd = 1.8;
	m_fAfvcc = 1.8;
	m_fVpp = 5.0;

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


	char tmpchar[50];
	sprintf(tmpchar, "%.6f", m_fAvdd);
	msg(tmpchar);

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

	msg("The Hz is: %f\n", m_fMclk);
	m_fMclk = 10.0;
	iRet = SetSensorClock(TRUE, (USHORT)(m_fMclk * 10), m_nDevID);
	if (iRet != DT_ERROR_OK)
	{
		msg("Set MCLK failed with err:%d\n", iRet);
	}
	Sleep(50);
	msg("The Hz is: %f\n", m_fMclk);

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
			// 每次采集为一簇800帧
			if (bOrgImage) // What, 判断出来都一样？？？
			{
				m_uFrameCnt++;
				m_uImageBytes = uDataSize;
			}
			else
			{
				m_uFrameCnt++;
				m_uImageBytes = uDataSize;
			}
			
			DataProc(bOrgImage, pBuffer, uDataSize, pOutBuffer, iWidth, iHeight);
		}
		
		/*
		switch (iRet) {
		case DT_ERROR_OK:
			msg("DT_ERROR_OR\n");
			break;
		case DT_ERROR_TIME_OUT:
			msg("DT_ERROR_TIME_OUT\n");
			break;
		case DT_ERROR_FAILED:
			msg("DT_ERROR_FAILED\n");
			break;
		case DT_ERROR_INTERNAL_ERROR:
			msg("DT_ERROR_INTERNAL_ERROR\n");
			break;
		}
		*/
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
void CMFCApplication1Dlg::Display_image_from_camera(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight)
{

	int i, j;
	BYTE *p;
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

	// 显示图像
	BYTE *pRgb = new BYTE[iWidth*iHeight * 3];
	pOut = (USHORT*)pOutBuffer;
	p = pRgb;
	if (pRgb != NULL)
	{
		// 转RGB图像
		for (i = 0; i < iPixels; i++)
		{
			*p = (pOut[i] * 2 > 255) ? 255 : (pOut[i] * 2 & 0xff);
			*(p + 1) = *p;
			*(p + 2) = *p;
			p += 3;
		}
		if (m_bSave)
		{
			if (cunum < cu_e)
			{
				for (i = 0; i < 400 * 250; i++)
				{
					//STOREAGE[cunum * 400 * 250 + i] = pRgb[3 * i];
				}
				cunum++;
			}
			if (cunum == cu_e)
			{
				msg("数据保存完成，请点击stop按钮");
				cunum++;
				m_bSave = FALSE;
			}
		}
		DrawImage(pRgb, iWidth, iHeight);

		delete[] pRgb;
	}
}
void CMFCApplication1Dlg::Display_image_byself(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight)
{
	int i, j;
	BYTE *p;
	int iPixels = iWidth * iHeight;
	USHORT *pOut = (USHORT*)pOutBuffer;

	//USHORT * dbuffer = new USHORT[iHeight*iWidth];
	//memset(dbuffer, 0, iHeight*iWidth * 2);

	uint itemp = 0;
	uint iframe = 0;
	uint frame;
	uint framecount;
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
	uchar *pulse = pInData;
	Mat img(Size(iWidth, iHeight), CV_8UC1);
	uchar *dbuffer = img.data;
	memset(dbuffer, 0x0, iHeight*iWidth);
	for (iframe = 0; iframe * 50 * 250 < length; iframe++)
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
	resize(img, img, Size(rt_rect.Width(), rt_rect.Height()));
	flip(img, img, 0);
	imshow("RT", img);
	//waitKey(5);
	/*
	BYTE *pRgb = new BYTE[iWidth*iHeight * 3];

	pOut = (USHORT*)dbuffer;
	p = pRgb;

	if (pRgb != NULL)
	{
		// 转RGB图像
		for (i = 0; i < iPixels; i++)
		{
			*p = (pOut[i] * 2 > 255) ? 255 : (pOut[i] * 2 & 0xff);
			*(p + 1) = *p;
			*(p + 2) = *p;
			p += 3;
		}

		DrawImage(pRgb, iWidth, iHeight);



	}
	//msg("Display using data processed by self\n");

	delete[] dbuffer;
	*/
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

	uchar* pulse = pInData;  //length为像素个数/8

	uint* interval = new uint[250 * 400];
	uint* label = new uint[250 * 400];
	uint* gray_first = new uint[250 * 400];
	uchar* gray = new uchar[250 * 400];


	for (int jj = 0; jj < 250 * 40 / 8; jj++)
	{
		label[jj] = 0x00;
	}


	uint itemp = 0;
	uint iframe = 0;
	uint frame;
	uint framecount;
	uint irow = 0;
	uint itime = 0;

	

	for (iframe = 0; iframe < FRAME_CUSUM_CNT; iframe++)
	{
		frame = iframe;
		for (irow = 0; irow < 250; irow++)
		{
			for (itime = 0; itime < 50; itime++)//每行52个字节
			{

				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q1)
				{
					interval[8 * 50 * irow + 8 * itime] = frame - label[8 * 50 * irow + 8 * itime];
					label[8 * 50 * irow + 8 * itime] = frame;
				}
				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q2)
				{
					interval[8 * 50 * irow + 8 * itime + 1] = frame - label[8 * 50 * irow + 8 * itime + 1];
					label[8 * 50 * irow + 8 * itime + 1] = frame;
				}
				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q3)
				{
					interval[8 * 50 * irow + 8 * itime + 2] = frame - label[8 * 50 * irow + 8 * itime + 2];
					label[8 * 50 * irow + 8 * itime + 2] = frame;
				}
				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q4)
				{
					interval[8 * 50 * irow + 8 * itime + 3] = frame - label[8 * 50 * irow + 8 * itime + 3];
					label[8 * 50 * irow + 8 * itime + 3] = frame;
				}
				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q5)
				{
					interval[8 * 50 * irow + 8 * itime + 4] = frame - label[8 * 50 * irow + 8 * itime + 4];
					label[8 * 50 * irow + 8 * itime + 4] = frame;
				}
				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q6)
				{
					interval[8 * 50 * irow + 8 * itime + 5] = frame - label[8 * 50 * irow + 8 * itime + 5];
					label[8 * 50 * irow + 8 * itime + 5] = frame;
				}
				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q7)
				{
					interval[8 * 50 * irow + 8 * itime + 6] = frame - label[8 * 50 * irow + 8 * itime + 6];
					label[8 * 50 * irow + 8 * itime + 6] = frame;
				}
				if (pulse[frame * 50 * 250 + irow * 50 + itime] & q8)
				{
					interval[8 * 50 * irow + 8 * itime + 7] = frame - label[8 * 50 * irow + 8 * itime + 7];
					label[8 * 50 * irow + 8 * itime + 7] = frame;
				}
			}
		}
		for (int mm = 0; mm < 250 * 400; mm++)
		{
			if (interval[mm] != 0)
			{
				gray_first[mm] = (3200 / interval[mm]> 255) ? 255 : ((3200 / interval[mm]) & 0xff);;
				gray[mm] = gray_first[mm];
			}
			if (interval[mm] == 0)
			{
				gray[mm] = 0;
			}
		}
		memcpy(slowdata + iframe * 400 * 250, gray, 400 * 250);
	}

}

void CMFCApplication1Dlg::Display_slow(int iWidth, int iHeight)
{
	Mat img(Size(iWidth, iHeight), CV_8UC1);
	uchar *data = img.data;
	//memset(data, 0x0, iHeight*iWidth);

	memcpy(data, slowdata + (f_cachebit_count + 200)*iWidth*iHeight, iWidth*iHeight);
	
	resize(img, img, Size(sl_rect.Width(), sl_rect.Height()));
	flip(img, img, 0);
	imshow("SL", img);

}

void CMFCApplication1Dlg::DataProc(BOOL bOrgImg, BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight)
{

	if (!bOrgImg)
	{
		Display_image_from_camera(pInData, uDataSize, pOutBuffer, iWidth, iHeight);
		
	}
	else
	{
		Display_image_byself(pInData, uDataSize, pOutBuffer, iWidth, iHeight);
	
		// 离散脉冲展示和慢速展示
		if (f_cachebit) {
			f_cachebit = FALSE;
			Display_slow_data(pInData, uDataSize, pOutBuffer, iWidth, iHeight);
		}
		Display_slow(iWidth,iHeight);
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
	m_strIniFile = ".\\40kfps.ini";
	enum_dev();
	open_dev();
	
	start_dev();
	
	if (!m_bRunning) {
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
		str.Format("FrameCount: %u, Rate: %.3ffps\r\nImageSize(BYTES): %u\r\n",
			m_uFrameCnt, m_fFrameRate, m_uImageBytes);
		
		m_fps.SetWindowText(str);
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
	m_bSave = FALSE;
	m_bRunning = FALSE;
	WaitForSingleObject(m_hThread, INFINITE);
	m_hThread = INVALID_HANDLE_VALUE;
	
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
