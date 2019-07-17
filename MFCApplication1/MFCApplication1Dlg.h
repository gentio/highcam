#pragma once
// MFCApplication1Dlg.h : ͷ�ļ�
//

#ifndef CAM_H
#define CAM_H



#include "DTCCM2_SDK\dtccm2.h"

#define WM_MSG	WM_USER+1

#define FRAME_CUSUM_CNT	400				//ԭʼͼ���ۼӺ����ã�����Ϊ100֡��100-800�ɵ�
#define WIDTH 400
#define HEIGHT 250

// CMFCApplication1Dlg �Ի���
//using namespace std;
//using namespace cv;

class img_buffer {
public:
	img_buffer();
	img_buffer(const img_buffer &rhs);
	BYTE data[WIDTH*HEIGHT];
};

class CMFCApplication1Dlg : public CDialogEx
{
// ����
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CMFCApplication1Dlg();
	

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;


	// ��ؼ���صı������������ؿؼ�ֵ
	CRect rt_rect; // ʵʱչʾ���ڴ�С
	CRect pl_rect; // ����չʾ
	CRect sl_rect; // ����չʾ
	CString m_strIniPathName;
	CEdit m_editStatus;
	CEdit m_editMsg;
	CString m_strIniFile;
	CComboBox m_wndDevList;
	
	CStatic m_wndVideo;
	CStatic m_slow;

	CStatic m_fps;
	CStatic m_rawdata;
	//CStatic m_slowimg;


	// һЩ�����صļ�¼
	int     m_nDevID;
	UINT    m_uFrameCnt;
	UINT    m_uImageBytes;
	FrameInfoEx m_frameInfo;
	float   m_fFrameRate;
	UINT    m_uFrameCntLast;
	BOOL    m_bOriginalImage;
	USHORT  m_uFrameCusum;

	// �����������
	float m_fAvdd;
	float m_fDovdd;
	float m_fDvdd;
	float m_fAfvcc;
	float m_fVpp;
	float m_fMclk;
	float m_fSavetime;  //  ���������

	float m_slowrates;
	UINT  slow_rates = 5;

	int   m_package_count; // Ҫ����İ�����
	int   m_temp_count = 0; // ����չʾ��ԭʼ���ݻ��������
	int   m_time_count; // ���ټ������
	int f_disp_location = 0;
	int frame_offset = 0;

	int f_cachebit_count;
	// ���ٴ��ڷ����ı���
	int slow_rate = 10;
	

	// �ж�������߳��Ƿ������еı�־����
	BOOL    m_Save_Package;   // ���Ʊ���raw�ı�־����
	BOOL    m_bOpen;
	BOOL    m_bRunning;

	BOOL    m_rt_display = FALSE;
	BOOL    m_bitproc = FALSE;
	BOOL    m_bitdisp = FALSE;
	BOOL    m_raw2video = FALSE;
	BOOL    m_slow_proc = FALSE;
	BOOL    m_display_slow = FALSE;
	BOOL f_cachebit = FALSE;

	BOOL f_display = FALSE;

	// �ĸ��������ߴ������ݵı�־����
	BOOL    f_save_slow_video = TRUE;
	BOOL    f_save_slow_video_bit = FALSE;
	BOOL    f_save_slow_img = FALSE;
	BOOL    f_save_slow_img_bit = FALSE;


	// ����ִ�в�ͬ���ܵ��߳�
	HANDLE  m_hThread;
	HANDLE  m_hThread_rt; //�Լ���ʱ�䴰��չʾ�߳� ��¼
	HANDLE  m_hThread_slow_data = INVALID_HANDLE_VALUE; // �������ݴ����߳� ��¼
	HANDLE  m_hThread_slow_display = INVALID_HANDLE_VALUE; // �Լ������崰��չʾ�߳� ��¼
	
	HANDLE  m_hThread_save_raw; // �����̵߳�״̬��¼
	HANDLE  m_hThread_raw2video = INVALID_HANDLE_VALUE; //�����̼߳�¼

	// һЩ�������� �¼���־����
	HANDLE  Mutex_rt;  // ��ȡ�����߳���ʵʱ����չʾ�̵߳Ļ�����
	HANDLE  Mutex_save;
	HANDLE  Mutex_deque;

	HANDLE  Event_rt = INVALID_HANDLE_VALUE;
	HANDLE  Event_save = INVALID_HANDLE_VALUE;
	HANDLE  Event_slow = INVALID_HANDLE_VALUE;





	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnMsg(WPARAM wP, LPARAM lP);
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:

	int enum_dev();
	int open_dev();
	int start_dev();

	void WorkProc();
	void load_and_proc();
	void save_raw_data();

	

	void DataProc(BOOL bOrgImg, BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_image_from_camera(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	
	void rt_display(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void procdata_for_slow(BYTE *pInData, ULONG uDataSize, BYTE *pOutBuffer, int iWidth, int iHeight);
	void Display_slow(int iWidth, int iHeight);
	void slow_display();

	afx_msg void Found_Cam();
	afx_msg void OnBnClickedCam();
	afx_msg void SetSaveTime();
	afx_msg void raw2video();
	afx_msg void SetPower();
	afx_msg void SetFreq();


private:

	

	BYTE *slowdata = NULL ;
	BYTE *slowdata_bit = NULL;
	BYTE *raw_data = NULL; // ����ԭʼ���ݰ��Ļ�����
	BYTE *save_data_buffer = NULL;

	BYTE *temp_data_buffer = NULL;
	BYTE *temp_disp_data_buffer = NULL;
	BYTE *temp_disp_bit_buffer = NULL;


public:
	afx_msg void set_slow_rates();
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