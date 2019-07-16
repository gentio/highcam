
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


class cparamlist param_buffer;

#pragma comment (lib,".\\DTCCM2_SDK\\dtccm2.lib")

extern void msg(LPCSTR lpszFmt, ...);
extern UINT __stdcall get_data_thread(LPVOID param);
extern UINT __stdcall rt_display_thread(LPVOID param);
extern UINT __stdcall slow_data_thread(LPVOID param);
extern UINT __stdcall save_raw_thread(LPVOID param);
extern UINT __stdcall offline_proc_thread(LPVOID param);
extern void getFiles(string path, vector<string>& files);

deque<img_buffer> img_que;
deque<img_buffer> img_que_bit;

img_buffer::img_buffer()
{
	for (int i = 0; i < WIDTH*HEIGHT; i++)
		data[i] = 0;
}

img_buffer::img_buffer(const img_buffer &rhs)
{
	for (int i = 0; i < WIDTH*HEIGHT; i++)
		data[i] = rhs.data[i];
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








CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
	, m_strIniFile(_T(".\\40kfps.ini"))
	, m_bOpen(FALSE)
	, m_nDevID(0)
	, m_bRunning(FALSE)
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_hThread_save_raw(INVALID_HANDLE_VALUE)
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

	raw_data = new BYTE[FRAME_CUSUM_CNT * 250 * 80]; // 用来进行实时处理的原始数据包的缓冲区
	save_data_buffer = new BYTE[FRAME_CUSUM_CNT * 250 * 80];

	temp_data_buffer = new BYTE[5*FRAME_CUSUM_CNT * 250 * 80]; //慢速展示数据的缓冲区
	//temp_disp_data_buffer = new BYTE[10*FRAME_CUSUM_CNT * 250 * 400];
	//temp_disp_bit_buffer = new BYTE[10 * FRAME_CUSUM_CNT * 250 * 400];
}
CMFCApplication1Dlg::~CMFCApplication1Dlg()
{
	/* 如果相机没有手动关闭而直接关闭应用程序，那么下次打开程序的时候
	图像恢复出来会出现间隔的条纹，但是在程序关闭的时候手动执行关闭相机会让程序等待太久的情况，
	这个时候会出现类似windows关机时候而直接杀掉程序的情况，这个时候相机也是没能自动关闭
	现在，默认关闭程序不要关闭相机
	*/

	/*
	if (m_bRunning) {


		m_bRunning = FALSE;

		WaitForSingleObject(m_hThread, INFINITE);
		m_rt_display = FALSE;
		m_bitproc = FALSE;
		WaitForSingleObject(m_hThread_slow_display, INFINITE);
		WaitForSingleObject(m_hThread_rt, INFINITE);

		int iRet = CloseDevice(m_nDevID);
		if (iRet != DT_ERROR_OK)
		{
			MessageBox("ERROR!!! Can't close the ");
		}
	}
	*/
	delete[] slowdata;
	delete[] slowdata_bit;
	delete[] raw_data;
	delete[] save_data_buffer;
	delete[] temp_data_buffer;
	delete[] temp_disp_data_buffer;
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

	DDX_Text(pDX, AVDD, m_fAvdd);
	DDX_Text(pDX, DOVDD, m_fDovdd);
	DDX_Text(pDX, DVDD, m_fDvdd);
	DDX_Text(pDX, AFVCC, m_fAfvcc);
	DDX_Text(pDX, VPP, m_fVpp);

	DDX_Text(pDX, MCLK, m_fMclk);
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
	ON_BN_CLICKED(CLOSE_CAM2, &CMFCApplication1Dlg::SetPower)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::SetFreq)
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


	Mutex_rt = CreateMutex(NULL, FALSE, NULL);
	Mutex_save = CreateMutex(NULL, FALSE, NULL);
	Mutex_deque = CreateMutex(NULL, FALSE, NULL);

	Event_rt = CreateEvent(NULL, FALSE, FALSE, _T("RT"));
	Event_save = CreateEvent(NULL, FALSE, FALSE, _T("SAVE"));
	Event_slow = CreateEvent(NULL, FALSE, FALSE, _T("slow"));
	


	// 500毫秒的定时器，用作状态刷新
	SetTimer(0, 500, NULL);
	// 用于统计帧率
	SetTimer(1, 4000, NULL);
	// 用于统计慢速展示的缓存区拷贝
	SetTimer(2, 5000, NULL);
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
		if (!enum_dev())
		{
			open_dev();
			start_dev();
		}
			
	}
	
	if (!m_bRunning) {
		//OnBnClickedCam();
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

	}
	else if (nIDEvent == 1)
	{
		m_fFrameRate = (float)(m_uFrameCnt - m_uFrameCntLast) / 4;
		m_uFrameCntLast = m_uFrameCnt;
	}
	else if (nIDEvent == 2)
	{
		m_temp_count = 5;
		frame_offset = 0;
		
	}
	
	CDialogEx::OnTimer(nIDEvent);
}

void CMFCApplication1Dlg::OnBnClickedCam()
{
	if (!m_bRunning) {
		MessageBox("相机未在工作，无需关闭\n");
	}
	else {
	
		m_bRunning = FALSE;
		SetEvent(Event_rt);
		WaitForSingleObject(m_hThread, INFINITE);
		ResetEvent(Event_rt);

		m_rt_display = FALSE;
		WaitForSingleObject(m_hThread_rt, INFINITE);

		m_slow_proc = FALSE;
		SetEvent (Event_slow);
		m_slow_proc = FALSE;
		WaitForSingleObject(m_hThread_slow_data, INFINITE);
		ResetEvent(Event_rt);

		m_display_slow = FALSE;
		WaitForSingleObject(m_hThread_slow_display, INFINITE);
			
	

		

		m_hThread = INVALID_HANDLE_VALUE;
		m_hThread_rt = INVALID_HANDLE_VALUE;
		m_hThread_slow_data = INVALID_HANDLE_VALUE;
		m_hThread_slow_display = INVALID_HANDLE_VALUE;

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


void CMFCApplication1Dlg::raw2video()
{
	if (m_raw2video) {
		MessageBox("The offline thread is running");
	}
	else {
		m_raw2video = TRUE;
		//if (m_hThread_raw2video == INVALID_HANDLE_VALUE)
			m_hThread_raw2video = (HANDLE)_beginthreadex(NULL, 0, &offline_proc_thread, this, 0, 0);

	}


	// TODO: 在此添加控件通知处理程序代码
}







