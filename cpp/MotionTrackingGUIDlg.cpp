
// MotionTrackingGUIDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MotionTrackingGUI.h"
#include "MotionTrackingGUIDlg.h"
#include "afxdialogex.h"
#include <opencv2/opencv.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMotionTrackingGUIDlg 대화 상자



CMotionTrackingGUIDlg::CMotionTrackingGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MOTIONTRACKINGGUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMotionTrackingGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	// --- IP 주소 표시 연결 (Static Text) ---
	DDX_Text(pDX, IDC_STATIC12, m_strIP_10); // ID 10 IP 주소
	DDX_Text(pDX, IDC_STATIC13, m_strIP_11); // ID 11 IP 주소

	// --- IP 10 축 (추종 축) 모니터링 ---
	DDX_Text(pDX, IDC_EDIT1, m_lCmdPos);
	DDX_Text(pDX, IDC_EDIT2, m_lActPos);
	DDX_Text(pDX, IDC_EDIT3, m_lActVel_pps);
	DDX_Text(pDX, IDC_EDIT4, m_dActVel_RPM); // RPM은 double 변수와 연결
	DDX_Text(pDX, IDC_EDIT5, m_lPosErr);

	// --- IP 11 축 (등속 축) 모니터링 ---
	DDX_Text(pDX, IDC_EDIT6, m_lCmdPos_11);
	DDX_Text(pDX, IDC_EDIT7, m_lActPos_11);
	DDX_Text(pDX, IDC_EDIT8, m_lActVel_pps_11);
	DDX_Text(pDX, IDC_EDIT9, m_dActVel_RPM_11); // RPM은 double 변수와 연결
	DDX_Text(pDX, IDC_EDIT10, m_lPosErr_11);

	// --- IP 주소 표시 (IDC_STATIC으로 가정) ---
	// IDC_STATIC_IP_X와 IDC_STATIC_IP_Y를 멤버 변수 m_strIP_X, m_strIP_Y와 연결하는 로직 필요
	// (현재 ID가 IDC_STATIC으로만 되어 있어 정확한 DDX 연결은 GUI에서 확인 후 필요)
	DDX_Control(pDX, IDC_BUTTON10, m_btnJogUp10);
	DDX_Control(pDX, IDC_BUTTON6, m_btnJogDown10);
	DDX_Control(pDX, IDC_BUTTON8, m_btnJogUp11);
	DDX_Control(pDX, IDC_BUTTON9, m_btnJogDown11);

	DDX_Control(pDX, IDC_COMBO1, m_cbOriginMethod); // 콤보박스 연결
}

BEGIN_MESSAGE_MAP(CMotionTrackingGUIDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CMotionTrackingGUIDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON7, &CMotionTrackingGUIDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON11, &CMotionTrackingGUIDlg::OnBnClickedButton11)

	ON_BN_CLICKED(IDC_BUTTON12, &CMotionTrackingGUIDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &CMotionTrackingGUIDlg::OnBnClickedButton13)

	ON_BN_CLICKED(IDC_BUTTON3, &CMotionTrackingGUIDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CMotionTrackingGUIDlg::OnBnClickedButton4)
	ON_WM_TIMER()

	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CMotionTrackingGUIDlg 메시지 처리기

BOOL CMotionTrackingGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// 1. IP 주소 초기화 및 화면에 표시
	m_strIP_10 = m_Controller.GetDriveIP(BD_ID_10).c_str();
	m_strIP_11 = m_Controller.GetDriveIP(BD_ID_11).c_str();

	// 2. 화면 업데이트 (IP 주소 및 초기 모니터링 값 표시)
	UpdateData(FALSE);

	// 3. 각 버튼에 ID와 방향 설정
	m_btnJogUp10.SetMotionParams(BD_ID_10, CW);
	m_btnJogDown10.SetMotionParams(BD_ID_10, CCW);

	m_btnJogUp11.SetMotionParams(BD_ID_11, CW);
	m_btnJogDown11.SetMotionParams(BD_ID_11, CCW);

	// 4. 카메라 초기화
	if (!m_Camera.InitializeCamera()) {
		printf("Camera Init Failed\n");
	}

	// 5. 타이머 설정 (현재 카메라 30FPS에 맞추기 위해 33)
	// 33ms마다 모니터링 데이터도 업데이트
	SetTimer(TIMER_ID_MONITOR, 33, NULL);
	UpdateData(FALSE);

	// 원점 복귀 방식 콤보박스 초기화
	m_cbOriginMethod.AddString(_T("0: Origin Sensor"));
	m_cbOriginMethod.AddString(_T("1: Z-Pulse"));
	m_cbOriginMethod.AddString(_T("2: Limit Sensor")); // 기존 기본값
	m_cbOriginMethod.AddString(_T("3: Z Limit"));
	m_cbOriginMethod.AddString(_T("4: Set Origin"));   // 현재 위치를 원점으로
	m_cbOriginMethod.AddString(_T("5: Z Phase"));
	m_cbOriginMethod.AddString(_T("6: Torque Origin"));
	m_cbOriginMethod.AddString(_T("7: Torque + Z"));

	m_cbOriginMethod.SetCurSel(0); // 기본값 2번 선택

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMotionTrackingGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMotionTrackingGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// IP 10 서보 ON 버튼
void CMotionTrackingGUIDlg::OnBnClickedButton7()
{
	// C++ 로직 호출: IP 10 축 연결 및 서보 ON
	m_Controller.ConnectionAndServoToggle(BD_ID_10, true);
	// TODO: 버튼 색상 변경 로직 추가
}

// IP 10 서보 OFF 버튼
void CMotionTrackingGUIDlg::OnBnClickedButton11()
{
	// C++ 로직 호출: IP 10 축 서보 OFF 및 연결 해제
	m_Controller.ConnectionAndServoToggle(BD_ID_10, false);
	// TODO: 버튼 색상 변경 로직 추가
}

// IP 11 서보 ON 버튼
void CMotionTrackingGUIDlg::OnBnClickedButton12()
{
	// C++ 로직 호출: IP 11 축 연결 및 서보 ON
	m_Controller.ConnectionAndServoToggle(BD_ID_11, true);
	// TODO: 버튼 색상 변경 로직 추가
}

// IP 11 서보 OFF 버튼
void CMotionTrackingGUIDlg::OnBnClickedButton13()
{
	// C++ 로직 호출: IP 11 축 서보 OFF 및 연결 해제
	m_Controller.ConnectionAndServoToggle(BD_ID_11, false);
	// TODO: 버튼 색상 변경 로직 추가.
}


// 원점 복귀 버튼
void CMotionTrackingGUIDlg::OnBnClickedButton3()
{
	// 1. 콤보박스에서 선택된 인덱스(0~7)를 가져옵니다.
	int nMethod = m_cbOriginMethod.GetCurSel();

	if (nMethod < 0) nMethod = 0; // 선택 안됐을 경우 기본값

	// 2. 선택된 방식으로 원점 복귀 시작 (별도 스레드 권장하나 여기선 직접 호출 예시)
	m_Controller.StartOriginSearch(nMethod);
}


// 트래킹 시작 버튼
void CMotionTrackingGUIDlg::OnBnClickedButton2()
{
	m_Controller.StartTracking();
}
// 트래킹 중단 버튼
void CMotionTrackingGUIDlg::OnBnClickedButton4()
{
	m_Controller.StopTracking();
}


void CMotionTrackingGUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	// 모니터링 타이머인지 확인
	if (nIDEvent == TIMER_ID_MONITOR)
	{
		// 카메라 프레임 캡처 및 출력
		m_Camera.CaptureAndProcessFrame();

		// Picture Control의 HDC와 영역(Rect)을 가져옵니다.
		CWnd* pCameraWnd = GetDlgItem(IDC_STATIC2);

		if (pCameraWnd) {
			CDC* pDC = pCameraWnd->GetDC();
			CRect rect;
			pCameraWnd->GetClientRect(&rect);

			// 조건문 없이 호출해야 "No Camera" 문구가 뜹니다.
			m_Camera.DrawFrameToHDC(pDC->GetSafeHdc(), rect);

			pCameraWnd->ReleaseDC(pDC);
		}

		long cmdPos, actPos, actVel_pps, posErr;

		// ID 10 (추종 축) 데이터 조회
		// Cmd Pos, Act Pos, Act Vel [pps], Pos Error 데이터를 가져옵니다.
		if (m_Controller.GetMotionStatus(BD_ID_10, cmdPos, actPos, actVel_pps, posErr))
		{
			// 2. RPM 계산 ([pps] -> [RPM])
			// 모터 분해능 10000 P/R, 리드 10mm 가정 -> 10000 펄스당 1회전
			m_dActVel_RPM = (double)actVel_pps * 60.0 / 10000.0;

			// 3. 멤버 변수 업데이트 (Edit Control 연결 변수)
			m_lCmdPos = cmdPos;
			m_lActPos = actPos;
			m_lActVel_pps = actVel_pps;
			m_lPosErr = posErr;

		}

		// ID 11 (등속 축) 데이터 조회
		// Cmd Pos, Act Pos, Act Vel [pps], Pos Error 데이터를 가져옵니다.
		if (m_Controller.GetMotionStatus(BD_ID_11, cmdPos, actPos, actVel_pps, posErr))
		{
			// 1. ID 11 전용 멤버 변수에 할당
			m_lCmdPos_11 = cmdPos;
			m_lActPos_11 = actPos;
			m_lActVel_pps_11 = actVel_pps;
			m_lPosErr_11 = posErr;
			m_dActVel_RPM_11 = (double)actVel_pps * 60.0 / 10000.0; // RPM 계산
		}
		// GUI 화면 업데이트 (DDX_Text로 연결된 모든 Edit Control을 갱신)
		UpdateData(FALSE);
	}

	CDialogEx::OnTimer(nIDEvent);
}
