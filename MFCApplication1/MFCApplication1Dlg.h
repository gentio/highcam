#pragma once
// MFCApplication1Dlg.h : 头文件
//

#ifndef CAM_H
#define CAM_H



#include "DTCCM2_SDK\dtccm2.h"

#define WM_MSG	WM_USER+1

#define FRAME_CUSUM_CNT	400				//原始图像累加和设置，步进为100帧，100-800可调

// CMFCApplication1Dlg 对话框
class CMFCApplication1Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// 标准构造函数
	~CMFCApplication1Dlg();
												// 离散脉冲慢速展示的判断
	int f_cachebit_count;
	BOOL f_cachebit = FALSE;
	
	// 枚举设备 打开设备，采集数据
	int enum_dev();
	int open_dev();
	int start_dev();
	
	void Display_image_from_camera(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_image_byself(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_slow_data(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_slow(int iWidth, int iHeight);

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	void DataProc(BOOL bOrgImg, BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);

	CString m_strIniPathName;
	CEdit m_editStatus;
	CEdit m_editMsg;
	CString m_strIniFile;
	CComboBox m_wndDevList;
	CComboBox COMBOFPS;
	CStatic m_wndVideo;
	CStatic m_slow;

	CStatic m_fps;
	CStatic m_rawdata;
	CStatic m_slowimg;

	float m_fAvdd;
	float m_fDovdd;
	float m_fDvdd;
	float m_fAfvcc;
	float m_fVpp;
	float m_fMclk;

	float m_fSavetime;  //  保存毫秒数

	int   m_package_count; // 要保存的包计数
	BOOL  m_Save_Package;   // 控制保存raw的标志变量

	BOOL    m_bOpen;
	int     m_nDevID;

	// 四个判断线程是否运行的标志变量
	BOOL    m_bRunning;
	BOOL    m_rawproc = FALSE;
	BOOL    m_bitproc = FALSE;
	BOOL    m_raw2video = FALSE;

	// 四个保存离线处理数据的标志变量
	BOOL    f_save_slow_video = TRUE;
	BOOL    f_save_slow_video_bit = FALSE;
	BOOL    f_save_slow_img = FALSE;
	BOOL    f_save_slow_img_bit = FALSE;

	HANDLE  m_hThread;
	HANDLE  m_hThread_slow; // 自己的脉冲窗口展示线程 记录
	HANDLE  m_hThread_self; //自己的时间窗口展示线程 记录
	HANDLE  m_hThread_save; // 保存线程的状态记录
	HANDLE  m_hThread_raw2video = INVALID_HANDLE_VALUE; //离线线程记录

	HANDLE  m_Mutex;  // 读取数据线程与实时窗口展示线程的互斥量
	HANDLE  m_rawMutex;
	HANDLE  m_Event;
	HANDLE  m_rawEvent;
	

	UINT    m_uFrameCnt;
	UINT    m_uImageBytes;

	FrameInfoEx m_frameInfo;
	float   m_fFrameRate;
	UINT    m_uFrameCntLast;
	BOOL    m_bOriginalImage;

	USHORT  m_uFrameCusum;



	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnMsg(WPARAM wP, LPARAM lP);
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:

	CRect rt_rect; // 实时展示窗口大小
	CRect pl_rect; // 脉冲展示
	CRect sl_rect; // 缓慢展示
	void WorkProc();
	void load_and_proc();

	void save_raw_data();
	afx_msg void Found_Cam();
	afx_msg void OnBnClickedCam();
	afx_msg void SetSaveTime();
	
	afx_msg void raw2video();

	afx_msg void SetPower();
	afx_msg void SetFreq();
private:
	// 慢速窗口放慢的倍数
	int slow_rate = 10;

	BYTE *slowdata = NULL ;
	BYTE *slowdata_bit = NULL;
	BYTE *raw_data = NULL; // 分配原始数据包的缓冲区
	BYTE *save_data_buffer = NULL;



};

class cparamlist {
public:
	CMFCApplication1Dlg *windows;
	BYTE *pInData;
	ULONG uDataSize;
	BYTE *pOutBuffer;
	int iWidth;
	int iHeight;
};


#endif