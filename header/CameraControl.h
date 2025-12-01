#pragma once

#include <opencv2/opencv.hpp>
#include <windows.h> 

class CameraControl
{
public:
    // 1. 카메라 연결 및 초기화
    bool InitializeCamera();
    void CloseCamera();

    // 2. 프레임 캡처 및 좌표 처리 (트래킹 스레드에서 호출)
    bool CaptureAndProcessFrame(); 

    // 2. 화면 갱신 (GUI 스레드에서 호출)
    void DrawFrameToHDC(HDC hdc, CRect rect); // 그리기만 담당

    // 3. 상태 조회
    bool IsCameraOpen() const { return m_bIsCameraOpen; }

    // 4. 좌표 추출 함수 (트래킹에 필요)
    double GetTrackedCoordinate(); // 영상 처리 결과를 mm 단위로 반환 (BD_ID_10 추종축용)

private:
    // OpenCV 객체
    cv::VideoCapture m_cap;
    cv::Mat m_frame;

    // 상태 변수
    bool m_bIsCameraOpen = false;

    // Helper: cv::Mat을 MFC Picture Control에 그릴 수 있는 BITMAP 형태로 변환하는 함수
    void DrawImageToHDC(cv::Mat& img, HDC hdc, CRect rect);
};