#pragma once
#include <afxwin.h>
class CJogButton :
    public CButton
{
    DECLARE_DYNAMIC(CJogButton)

public:
    CJogButton();
    virtual ~CJogButton();

    // 축 ID와 방향을 설정하는 함수
    void SetMotionParams(int nBdID, int nDir);

private:
    int m_nBdID; // 제어할 모터 ID
    int m_nDir;  // 이동 방향 (CW/CCW)

protected:
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

