#include "pch.h"
#include "CameraControl.h"

// C++ 표준 라이브러리
#include <iostream>
#include <cstdlib>
#include <ctime>

// OpenCV 라이브러리
#include <opencv2/opencv.hpp> 

// 모터 제어 클래스 (상수 정의 포함)
#include "DriveControl.h" 

// OpenCV 네임스페이스 사용 선언
using namespace cv;

// C++ 표준 네임스페이스 사용 선언
using namespace std;

bool CameraControl::InitializeCamera()
{
    // 카메라 ID 0번 시도
    m_cap.open(0);

    if (m_cap.isOpened()) {
        m_bIsCameraOpen = true;

        // 캡처 속성 설정 (선택 사항: 예시로 640x480 설정)
        m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        printf("Camera opened.\n");
        return true;
    }

    m_bIsCameraOpen = false;
    printf("ERROR: Camera failed to open (ID 0).\n");
    return false;
}

void CameraControl::CloseCamera()
{
    if (m_cap.isOpened()) {
        m_cap.release();
        m_bIsCameraOpen = false;
    }
}

// 2. 프레임 캡처 및 처리 (인자 없음)
// 트래킹 스레드에서 호출되며, 화면 그리기와 무관하게 데이터만 갱신합니다.
bool CameraControl::CaptureAndProcessFrame()
{
    if (!m_bIsCameraOpen) return false;

    // 프레임 캡처하여 멤버 변수 m_frame에 저장
    if (m_cap.read(m_frame))
    {
        // (향후) 여기에 OpenCV 영상 처리 로직 구현
        // 예: cv::cvtColor, cv::findContours 등

        return true; // 캡처 성공
    }
    return false; // 캡처 실패
}

// 3. 화면 그리기 전용 함수 (GUI 스레드용)
// OnTimer에서 호출되며, 이미 캡처된 m_frame을 화면에 그립니다.
void CameraControl::DrawFrameToHDC(HDC hdc, CRect rect)
{
    // [수정] 카메라가 열리지 않았거나 프레임이 없는 경우 "No Camera" 표시
    if (!m_bIsCameraOpen || m_frame.empty()) {
        // 1. 배경을 검은색으로 채움
        ::FillRect(hdc, &rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));

        // 2. 텍스트 설정 (빨간색 글씨, 배경 투명)
        ::SetBkMode(hdc, TRANSPARENT);
        ::SetTextColor(hdc, RGB(255, 0, 0)); // 빨간색

        // 3. 화면 중앙에 텍스트 출력
        CString strMsg = _T("No Camera");
        ::DrawText(hdc, strMsg, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        return;
    }
    

    // 내부 헬퍼 함수 호출
    DrawImageToHDC(m_frame, hdc, rect);
}

// 4. 내부 헬퍼: OpenCV Mat -> MFC DC 변환 및 출력
void CameraControl::DrawImageToHDC(cv::Mat& img, HDC hdc, CRect rect)
{
    // 이미지 크기를 Picture Control 크기에 맞게 조정
    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(rect.Width(), rect.Height()));

    // BGR -> BGRA 변환 (Windows 호환성)
    if (resized_img.channels() == 3) {
        cv::cvtColor(resized_img, resized_img, cv::COLOR_BGR2BGRA);
    }

    // 비트맵 헤더 설정
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = resized_img.cols;
    bmi.bmiHeader.biHeight = -resized_img.rows; // Top-down 방식

    if (resized_img.channels() == 3 || resized_img.channels() == 4) {
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = (resized_img.channels() == 4) ? 32 : 24;
        bmi.bmiHeader.biCompression = BI_RGB;
    }

    // 화면에 그리기
    ::SetDIBitsToDevice(
        hdc,
        rect.left, rect.top,
        rect.Width(), rect.Height(),
        0, 0,
        0, resized_img.rows,
        resized_img.data,
        &bmi,
        DIB_RGB_COLORS
    );
}


// 5. 좌표 반환 함수
double CameraControl::GetTrackedCoordinate()
{
    // (현재는 임의의 좌표 생성 로직을 사용합니다.)

    // 이 위치에 cv::findContours, cv::moments 등을 사용하여
    // 물체의 중심 좌표를 계산하고, 이를 mm 단위로 변환하는 로직이 들어갑니다.

    return 25.0; // 임시로 25mm를 반환
}