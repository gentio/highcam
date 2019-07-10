
// MFCApplication1Dlg.h : ͷ�ļ�
//

#pragma once

#include "DTCCM2_SDK\dtccm2.h"

#define WM_MSG	WM_USER+1

#define FRAME_CUSUM_CNT	400				//ԭʼͼ���ۼӺ����ã�����Ϊ100֡��100-800�ɵ�

// CMFCApplication1Dlg �Ի���
class CMFCApplication1Dlg : public CDialogEx
{
// ����
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// ��׼���캯��
	// ��ɢ��������չʾ���ж�
	int f_cachebit_count;
	BOOL f_cachebit;
	//BYTE slowdata[400 * 250 * FRAME_CUSUM_CNT];
	//BYTE slowdata_bit[400 * 250 * FRAME_CUSUM_CNT];

	int enum_dev();
	int open_dev();
	int start_dev();
	void DrawImage(BYTE *pRgb, int iWidth, int iHeight);
	void DrawImage_slow(BYTE *pRgb, int iWidth, int iHeight);
	void Display_image_from_camera(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_image_byself(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_slow_data(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_slow(int iWidth, int iHeight);

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
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

	float m_fSavetime;

	int   m_package_count; // Ҫ����İ�����
	BOOL  m_Save_Package;   // ���Ʊ���raw�ı�־����

	BOOL    m_bOpen;
	int     m_nDevID;
	BOOL    m_bRunning;
	BOOL    m_rawproc = FALSE;
	BOOL    m_bitproc = FALSE;
	BOOL    m_raw2video = FALSE;
	BOOL    f_save_slow_video = TRUE;
	BOOL    f_save_slow_video_bit = FALSE;
	BOOL    f_save_slow_img = FALSE;
	BOOL    f_save_slow_img_bit = FALSE;

	HANDLE  m_hThread;
	HANDLE  m_hThread_slow; // �Լ������崰��չʾ�߳� ��¼
	HANDLE  m_hThread_self; //�Լ���ʱ�䴰��չʾ�߳� ��¼
	HANDLE  m_hThread_save; // �����̵߳�״̬��¼
	HANDLE  m_hThread_raw2video = INVALID_HANDLE_VALUE; //�����̼߳�¼

	HANDLE  m_Mutex;
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
	BOOL    m_bSave;
	BOOL    m_Video;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnMsg(WPARAM wP, LPARAM lP);
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:

	CRect rt_rect; // ʵʱչʾ���ڴ�С
	CRect pl_rect; // ����չʾ
	CRect sl_rect; // ����չʾ
	void WorkProc();
	void load_and_proc();

	void save_raw_data();
	afx_msg void Found_Cam();
	afx_msg void OnBnClickedCam();
	afx_msg void SetSaveTime();
	
	afx_msg void raw2video();
};
