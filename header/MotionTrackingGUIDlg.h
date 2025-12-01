
// MotionTrackingGUIDlg.h: 헤더 파일
//
#include "DriveControl.h"
#include "CJogButton.h"
#include "CameraControl.h"

#pragma once

#define TIMER_ID_MONITOR 100

// CMotionTrackingGUIDlg 대화 상자
class CMotionTrackingGUIDlg : public CDialogEx
{
// 생성입니다.
public:
	CMotionTrackingGUIDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MOTIONTRACKINGGUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	// 1. MotionController 객체 선언
	MotionController m_Controller;

private:

	// 2. IP 주소를 표시할 멤버 변수 (Static Text 컨트롤과 연결)
	CString m_strIP_10; // ID 10 (추종 축) IP
	CString m_strIP_11; // ID 11 (등속 축) IP

	// 3. 모니터링 데이터 (Edit Control과 연결)
	long m_lCmdPos, m_lActPos, m_lActVel_pps, m_lPosErr; // ID 10 (추종 축)
	long m_lCmdPos_11, m_lActPos_11, m_lActVel_pps_11, m_lPosErr_11; // ID 11 (등속 축)
	double m_dActVel_RPM, m_dActVel_RPM_11; // RPM은 double로 변환해야 함

	// 4. 카메라 관련 변수
	CameraControl m_Camera;

public:
	
	CJogButton m_btnJogUp10;
	CJogButton m_btnJogDown10;
	CJogButton m_btnJogUp11;
	CJogButton m_btnJogDown11;
	CComboBox m_cbOriginMethod; // 콤보박스 멤버 변수
};
