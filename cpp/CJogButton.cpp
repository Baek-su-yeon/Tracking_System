#include "pch.h"
#include "CJogButton.h"
#include "MotionTrackingGUIDlg.h" // 메인 다이얼로그 헤더 포함

// 1. 다이나믹 매크로 추가 (헤더에 DECLARE_DYNAMIC이 있으므로 짝을 맞춰야 함)
IMPLEMENT_DYNAMIC(CJogButton, CButton)

// 2. 생성자 구현 (빠져 있던 부분)
CJogButton::CJogButton()
{
}

// 3. 소멸자 구현 (빠져 있던 부분)
CJogButton::~CJogButton()
{
}

BEGIN_MESSAGE_MAP(CJogButton, CButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CJogButton::SetMotionParams(int nBdID, int nDir)
{
	m_nBdID = nBdID;
	m_nDir = nDir;
}

void CJogButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 부모 창(메인 다이얼로그)을 가져옴
	CMotionTrackingGUIDlg* pParent = (CMotionTrackingGUIDlg*)GetParent();

	// 부모 창의 MotionController를 통해 조그 시작 호출
	// (주의: Controller나 StartJogMove가 public이어야 함)
	if (pParent) {
		pParent->m_Controller.StartJogMove(m_nBdID, m_nDir);
	}

	// 버튼이 눌린 시각적 효과를 위해 기본 함수 호출
	CButton::OnLButtonDown(nFlags, point);
}


void CJogButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	// 부모 창 가져옴
	CMotionTrackingGUIDlg* pParent = (CMotionTrackingGUIDlg*)GetParent();

	// 조그 정지 호출
	if (pParent) {
		pParent->m_Controller.StopJogMove(m_nBdID);
	}
	CButton::OnLButtonUp(nFlags, point);
}
