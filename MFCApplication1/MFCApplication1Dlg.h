
// MFCApplication1Dlg.h : 头文件
//

#pragma once

#include "DTCCM2_SDK\dtccm2.h"

#define WM_MSG	WM_USER+1

// CMFCApplication1Dlg 对话框
class CMFCApplication1Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// 标准构造函数



	int enum_dev();
	int open_dev();
	int start_dev();
	void DrawImage(BYTE *pRgb, int iWidth, int iHeight);

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

	CStatic m_fps;

	float m_fAvdd;
	float m_fDovdd;
	float m_fDvdd;
	float m_fAfvcc;
	float m_fVpp;
	float m_fMclk;
	float m_fSavetime;

	BOOL    m_bOpen;
	int     m_nDevID;
	BOOL    m_bRunning;
	HANDLE  m_hThread;

	UINT    m_uFrameCnt;
	UINT    m_uImageBytes;

	FrameInfoEx m_frameInfo;
	float   m_fFrameRate;
	UINT    m_uFrameCntLast;
	BOOL    m_bOriginalImage;

	USHORT  m_uFrameCusum;
	BOOL    m_bSave;
	BOOL    m_Video;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnMsg(WPARAM wP, LPARAM lP);
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:
	void WorkProc();
	afx_msg void Found_Cam();
	afx_msg void OnBnClickedCam();
};
