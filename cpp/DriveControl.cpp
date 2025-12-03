#include "pch.h"

#include <iostream>
#include <cstdlib> // rand, srand

#include "FAS_EziMOTIONPlusE.h"
#include "ReturnCodes_Define.h"
#include "MOTION_DEFINE.h"
#include "DriveControl.h"
#include "CameraControl.h"


using namespace PE;
using namespace std;

// 실행부

// 함수부
// 드라이브 초기 설정
// 드라이브 IP 주소 반환
string MotionController::GetDriveIP(int bdID)
{
    if (bdID == BD_ID_10) {
        return m_ip10;
    }
    else if (bdID == BD_ID_11) {
        return m_ip11;
    }
    else {
        return "Error: Invalid ID";
    }
}


// 드라이브 통신 연결
bool MotionController::Connect(int bdID, int nCommType)
{
    // IP 주소를 클래스 변수에서 가져옵니다.
    BYTE byIP10[4] = { 192, 168, 0, 10 };
    BYTE byIP11[4] = { 192, 168, 0, 11 };

    BYTE* ip_to_use = (bdID == BD_ID_10) ? byIP10 : byIP11;

    // TCP 연결 시도
    if (nCommType == TCP) {
        if (FAS_ConnectTCP(ip_to_use[0], ip_to_use[1], ip_to_use[2], ip_to_use[3], bdID) == FALSE) {
            printf("ERROR: [%s] TCP Connect Fail!\n", GetDriveIP(bdID).c_str());
            return false;
        }
    }
    // UDP 연결 시도
    else if (nCommType == UDP) {
        if (FAS_Connect(ip_to_use[0], ip_to_use[1], ip_to_use[2], ip_to_use[3], bdID) == FALSE) {
            printf("ERROR: [%s] UDP Connect Fail!\n", GetDriveIP(bdID).c_str());
            return false;
        }
    }

    // 전체 연결 상태 (m_bIsConnected)는 두 축 모두 연결된 상태인지 확인하여 업데이트 필요
    // 이 함수 내에서는 단일 축 연결 성공만 반환
    printf("[%s] connected successfully.\n", GetDriveIP(bdID).c_str());
    return true;
}

// 드라이브 서보 ON
bool MotionController::SetServo(int bdID, bool bTurnOn)
{
    // 1. 초기 상태 확인
    EZISERVO2_AXISSTATUS AxisStatus;

    if (bTurnOn == true)
    {
        // if ServoOnFlagBit is OFF('0'), switch to ON('1')
        if (FAS_GetAxisStatus(bdID, &(AxisStatus.dwValue)) != FMM_OK)
        {
            printf("ERROR: [%s] Function(FAS_GetAxisStatus) was failed.\n", GetDriveIP(bdID).c_str());
            return false;
        }

        if (AxisStatus.FFLAG_SERVOON == 0)
        {
            // 2. Servo ON 명령
            if (FAS_ServoEnable(bdID, TRUE) != FMM_OK)
            {
                printf("ERROR: [%s] Function(FAS_ServoEnable) was failed.\n", GetDriveIP(bdID).c_str());
                return false;
            }


            // 3. Servo ON 상태 대기
            do
            {
                Sleep(1);
                if (FAS_GetAxisStatus(bdID, &(AxisStatus.dwValue)) != FMM_OK)
                {
                    printf("ERROR: [%s] Function(FAS_GetAxisStatus) was failed.\n", GetDriveIP(bdID).c_str());
                    return false;
                }
            } while (!AxisStatus.FFLAG_SERVOON);
        }
        printf("[%s] Servo ON complete.\n", GetDriveIP(bdID).c_str());
    }

    else
    {
        // Servo OFF 명령 실행
        if (FAS_ServoEnable(bdID, FALSE) != FMM_OK)
        {
            printf("ERROR: [%s] Function(FAS_ServoEnable(OFF)) was failed.\n", GetDriveIP(bdID).c_str());
            return false;
        }
    }


    // 상태 업데이트:
    if (bdID == BD_ID_10) m_bServoOn10 = bTurnOn;
    else if (bdID == BD_ID_11) m_bServoOn11 = bTurnOn;


    return true;
}

// GUI 버튼 연동 함수: 서보 ON/OFF 버튼
bool MotionController::ConnectionAndServoToggle(int bdID, bool bTurnOn)
{
    if (bTurnOn)
    {
        // 연결 시도 해보고 안되면 강종
        if (!Connect(bdID, TCP))
        {
            printf("Check HW power on");
            return false;
        }

        // 2. 해당 축 Servo ON
        return SetServo(bdID, true);
        printf("[%s] servo on and connected.\n", GetDriveIP(bdID).c_str());


    }
    else {
        // 해당 축 Servo OFF, 통신 연결 해제
        bool result = SetServo(bdID, false);

        if (result) FAS_Close(bdID);
        printf("[%s] servo off and disconnected.\n", GetDriveIP(bdID).c_str());
        return result;
    }
}


// 원점 이동 관련 함수
// 원점 복귀 시 파라미터 설정
bool MotionController::SetOriginParameters(int bdID, int nOriginMethod)
{
    // Set Origin Parameter (기본값)
    int nOrgSpeed = 50000; // 원점 복귀 시작 시 운전 속도 (파라미터 14번)
    int nOrgSearchSpeed = 1000; // 원점 센서 감지 후 저속 운전 속도 (파라미터 15번)
    int nOrgAccDecTime = 50; // 원점 복귀 시 가감속 구간의 할당 시간 (파라미터 16번)
    int nOrgMethod = nOriginMethod;	// 원점 복귀 방식 선택 (파라미터 17번)
    int nOrgDir = CW; // 원점 복귀 운전 시 모터의 회전 방향 (0: CW, 1: CCW) (파라미터 18번)
    int nOrgOffset = 0; // 원점 복귀 종료 후 추가 이동할 위치 펄스 값 (파라미터 19번)
    int nOrgPositionSet = 0; // 원점 복귀 종료 후 Command/Actual Pos에 설정할 값 (파라미터 20번)
    int nOrgTorqueRatio = 50;// Torque Origin 방식 사용 시 정지하기 위한 힘의 비율 (%) (파라미터 27번)

    if (FAS_SetParameter(bdID, SERVO2_ORGSPEED, nOrgSpeed) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGSPEED]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    if (FAS_SetParameter(bdID, SERVO2_ORGSEARCHSPEED, nOrgSearchSpeed) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGSEARCHSPEED]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    if (FAS_SetParameter(bdID, SERVO2_ORGACCDECTIME, nOrgAccDecTime) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGACCDECTIME]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    if (FAS_SetParameter(bdID, SERVO2_ORGMETHOD, nOrgMethod) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGMETHOD]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    if (FAS_SetParameter(bdID, SERVO2_ORGDIR, nOrgDir) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGDIR]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    if (FAS_SetParameter(bdID, SERVO2_ORGOFFSET, nOrgOffset) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGOFFSET]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    if (FAS_SetParameter(bdID, SERVO2_ORGPOSITIONSET, nOrgPositionSet) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGPOSITIONSET]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    if (FAS_SetParameter(bdID, SERVO2_ORGTORQUERATIO, nOrgTorqueRatio) != FMM_OK)
    {
        printf("ERROR: [%s] Function(FAS_SetParameter[SERVO2_ORGTORQUERATIO]) was failed.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    return true;
}

// GUI 버튼 연동 함수: 원점 복귀 버튼
bool MotionController::StartOriginSearch(int nOriginMethod)
{
    // 드라이브 둘 다 서보 켜져있는지 확인
    if (!m_bServoOn10 || !m_bServoOn11) {
        printf("ERROR: Not all axes are Servo ON. Origin search requires Servo ON.\n");
        return false;
    }

    int ids[] = { BD_ID_10, BD_ID_11 };

    printf("--- Starting Origin Search on both axes ---\n");

    for (int bdID : ids)
    {
        //1. 원점 파라미터 설정
        if (!SetOriginParameters(bdID, nOriginMethod)) {
            printf("ERROR: [%s] Failed to set origin parameters.\n", GetDriveIP(bdID).c_str());
            return false;
        }

        // 2. 원점 복귀 명령 (FAS_MoveOriginSingleAxis)
        if (FAS_MoveOriginSingleAxis(bdID) != FMM_OK) {
            printf("ERROR: [%s] FAS_MoveOriginSingleAxis failed.\n", GetDriveIP(bdID).c_str());
            return false;
        }
    }

    // 3. 완료 대기 (두 축이 모두 완료될 때까지 기다립니다)
    EZISERVO2_AXISSTATUS AxisStatus10, AxisStatus11;
    bool bOriginReturning = true;

    printf("Waiting for Origin Search to complete...\n");

    while (bOriginReturning)
    {
        Sleep(10); // 10ms 대기

        // ID 10 상태 확인
        FAS_GetAxisStatus(BD_ID_10, &(AxisStatus10.dwValue));
        // ID 11 상태 확인
        FAS_GetAxisStatus(BD_ID_11, &(AxisStatus11.dwValue));

        // 두 축 모두 원점 복귀 중(`FFLAG_ORIGINRETURNING`)이 아니거나, 
        // 완료(`FFLAG_ORIGINRETOK`) 플래그가 켜지면 루프 탈출
        bOriginReturning = AxisStatus10.FFLAG_ORIGINRETURNING || AxisStatus11.FFLAG_ORIGINRETURNING;

        if (AxisStatus10.FFLAG_ERRORALL || AxisStatus11.FFLAG_ERRORALL) {
            printf("CRITICAL ERROR: Alarm detected during Origin Search. Stopping.\n");
            return false;
        }
    }


    // 4. 완료 확인 (Method 4: Set Origin은 센서체크가 없으므로 OriginRetOK 플래그가 안뜰 수도 있음 [cite: 3033])
    // 따라서 Method 4가 아닐 때만 OriginRetOK를 체크하거나, 그냥 완료 메시지를 띄움
    if (nOriginMethod != 4) {
        if (AxisStatus10.FFLAG_ORIGINRETOK && AxisStatus11.FFLAG_ORIGINRETOK) {
            printf("Origin Search complete on both axes.\n");
            return true;
        }
        else {
            printf("ERROR: Origin Search failed to complete on both axes.\n");
            return false;
        }
    }
    else {
        printf("Origin Set (Method 4) complete.\n");
        return true;
    }

    return false;
}


// 드라이버 조종 함수
// GUI 버튼 연동 함수: 드라이버 이동 시작(화살표 버튼 눌림)
bool MotionController::StartJogMove(int bdID, int dir)
{
    // Servo ON 상태 확인
    if (!m_bServoOn10 && bdID == BD_ID_10) {
        printf("ERROR: [%s] Axis not Servo ON.\n", GetDriveIP(bdID).c_str());
        return false;
    }
    if (!m_bServoOn11 && bdID == BD_ID_11) {
        printf("ERROR: [%s] Axis not Servo ON.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    // dir: CW(1) - 위/오른쪽 방향, CCW(0) - 아래/왼쪽 방향
    int nDir = (dir == CW) ? CW : CCW;

    if (ADJUST_ACC == 0)
    {
        if (FAS_MoveVelocity(bdID, ADJUST_VELOCITY, nDir) != FMM_OK)
        {
            printf("ERROR: [%s] FAS_MoveVelocity failed to start.\n", GetDriveIP(bdID).c_str());
            return false;
        }
    }
    else
    {
        // 가감속 옵션 설정
        VELOCITY_OPTION_EX opt = { 0 };
        opt.flagOption.BIT_USE_CUSTOMACCDEC = 1;
        opt.wCustomAccDecTime = ADJUST_ACC;

        if (FAS_MoveVelocityEx(bdID, ADJUST_VELOCITY, nDir, &opt) != FMM_OK)
        {
            printf("ERROR: [%s] FAS_MoveVelocityEx failed to start (Vel: %ld).\n",
                GetDriveIP(bdID).c_str(), ADJUST_VELOCITY);
            return false;
        }
    }

    printf("[%s] Jog Move Started (Dir: %s, Speed: %d pps).\n",
        GetDriveIP(bdID).c_str(), (nDir == CW ? "CW" : "CCW"), ADJUST_VELOCITY);

    return true;
}

// GUI 버튼 연동 함수: 드라이버 이동 멈춤(화살표 버튼 떼짐)
bool MotionController::StopJogMove(int bdID)
{
    // FAS_MoveStop: 운전 중인 모터를 감속하면서 정지
    if (FAS_MoveStop(bdID) != FMM_OK)
    {
        printf("ERROR: [%s] FAS_MoveStop failed to execute.\n", GetDriveIP(bdID).c_str());
        return false;
    }

    // Jog Move는 모터가 멈출 때까지 기다려야 하지만, GUI가 멈추는 것을 방지하기 위해 
    // 여기서는 간단히 명령만 보내고 즉시 반환합니다.
    printf("[%s] Jog Move Stopped.\n", GetDriveIP(bdID).c_str());
    return true;
}


// Tarcking 관련 함수
// 단위 환산 함수
long MotionController::MM_TO_PULSE(double mm) {
    return (long)(mm * PULSE_PER_MM);
}

// tarcking 좌표 추출 함수
double MotionController::GenerateTarget() {
    // 지금은 랜덤함수를 따라서 움직입니다.
    // 향후 영상처리를 통해 최종 좌표값(자료형:double, 단위:mm)을 출력할 수 있도록 변경이 필요합니다.
    // 영상 입력 받을 경우 인자 변경 필요
    double random_val = (double)rand() / RAND_MAX;
    return random_val * MAX_AXIS_RANGE_MM;
}

// 트래킹 기능
void MotionController::TrackingLoopThread()
{
    printf("Tracking Thread ID: %lu started.\n", GetCurrentThreadId());

    EZISERVO2_AXISSTATUS AxisStatus;

    // 카메라 연결 상태를 미리 확인
    bool bCameraAvailable = m_Camera.IsCameraOpen();

    while (m_bIsTracking) // StopTracking()에서 플래그가 false가 될 때까지 실행
    {
        // 1. 안전 상태 확인 (필수)
        if (FAS_GetAxisStatus(BD_ID_10, &(AxisStatus.dwValue)) != FMM_OK || AxisStatus.FFLAG_ERRORALL) {
            printf("CRITICAL ERROR in thread: Axis 10 Alarm Detected. Stopping tracking.\n");
            break;
        }
        if (FAS_GetAxisStatus(BD_ID_11, &(AxisStatus.dwValue)) != FMM_OK || AxisStatus.FFLAG_ERRORALL) {
            printf("CRITICAL ERROR in thread: Axis 11 Alarm Detected. Stopping tracking.\n");
            break;
        }

        // 현재는 GenerateTarget() (랜덤 시뮬레이션) 로직으로 실행
        double target_mm_10 = GenerateTarget();
        long target_pulse_10 = MM_TO_PULSE(target_mm_10);

        // 나중에 영상처리 코드 구현 시 사용, 위에 랜덤 로직은 삭제
        // 카메라 연결 상태에 따라 분기
        //if (bCameraAvailable)
        //{
        //    //// --- 2A. 카메라 연결 상태 (정상 트래킹 로직) ---
        //    continue

        //}
        //else
        //{
        //    // --- 2B. 카메라 연결 안 됨 (랜덤 시뮬레이션 또는 대기) ---
        //    
        //}

        Sleep(LOOP_DELAY_MS); // 50ms 대기
    }

    // 루프가 종료되면 (StopTracking() 호출 또는 알람 발생 시)
    // 플래그를 최종적으로 false로 설정 (혹시 모를 에러 종료 시)
    m_bIsTracking = false;

    printf("Tracking Thread ID: %lu terminated.\n", GetCurrentThreadId());
}

// Windows 스레드 진입점 (Static 함수)
// 스레드를 시작하려면 클래스 멤버 함수가 아닌 정적(static) 함수가 필요합니다.
DWORD WINAPI StaticTrackingLoopThread(LPVOID lpParam)
{
    // 스레드 매개변수에서 MotionController 객체 포인터를 가져옵니다.
    MotionController* pController = (MotionController*)lpParam;
    if (pController) {
        pController->TrackingLoopThread(); // 실제 루프 실행
    }
    return 0;
}

// GUI 버튼 연동 함수: 트래킹 ON
bool MotionController::StartTracking()
{
    // 트래킹 상태 확인
    if (m_bIsTracking) {
        printf("Tracking is already running.\n");
        return true;
    }

    // 서보 상태 확인
    if (!m_bServoOn10 || !m_bServoOn11) {
        printf("ERROR: Servo must be ON on both axes to start tracking.\n");
        return false;
    }

    printf("\n--- Tracking System Initializing Motion ---\n");

    // 1. ID 11 (등속 축): MoveVelocity 명령 시작 (기반 모션)
    if (FAS_MoveVelocity(BD_ID_11, CONST_VELOCITY, CW) != FMM_OK) {
        printf("ERROR: IP 11 (Const Vel) MoveVelocity Failed.\n");
        return false;
    }

    // 2. ID 10 (좌표 추종 축): Position Override를 위한 기반 모션 시작
    // 모터가 움직이는 상태여야 FAS_PositionAbsOverride가 작동
    // 현재 상태를 보기 위해 가장 처음에 initial_pos만큼 움직입니다.
    long initial_pos = MM_TO_PULSE(MAX_AXIS_RANGE_MM);
    if (FAS_MoveSingleAxisAbsPos(BD_ID_10, initial_pos, TRACKING_VELOCITY) != FMM_OK) {
        printf("ERROR: IP 10 (Tracking) Initial MoveAbsPos Failed.\n");
        FAS_MoveStop(BD_ID_11); // ID 11 안전 정지
        return false;
    }


    // 3. 트래킹 상태 플래그 ON 및 스레드 시작
    m_bIsTracking = true;
    m_hTrackingThread = CreateThread(NULL, 0, StaticTrackingLoopThread, this, 0, NULL);

    if (m_hTrackingThread == NULL) {
        printf("CRITICAL ERROR: Failed to create tracking thread.\n");
        FAS_MoveStop(BD_ID_10); FAS_MoveStop(BD_ID_11);
        m_bIsTracking = false;
        return false;
    }

    printf("SUCCESS: Tracking started on a new thread.\n");
    return true;
}

// GUI 버튼 연동 함수: 트래킹 OFF
bool MotionController::StopTracking()
{
    if (!m_bIsTracking) {
        printf("Tracking is already stopped.\n");
        return true;
    }

    printf("--- Stopping Tracking Loop ---\n");

    // 1. 스레드 종료 요청 (플래그 OFF)
    m_bIsTracking = false;
    // 2. 스레드 종료 대기 (최대 5초 대기)
    if (m_hTrackingThread != NULL) {
        WaitForSingleObject(m_hTrackingThread, 5000); // 5초 대기 후 강제 종료 방지
        CloseHandle(m_hTrackingThread);
        m_hTrackingThread = NULL;
    }

    // 3. 두 축 안전 정지
    FAS_MoveStop(BD_ID_10);
    FAS_MoveStop(BD_ID_11);

    printf("Tracking safely stopped and axes halted.\n");
    return true;
}

bool MotionController::GetMotionStatus(int bdID, long& cmdPos, long& actPos, long& actVel, long& posErr)
{
    // FAS_GetMotionStatus: CmdPos, ActPos, PosErr, ActVel, PT Item No.를 한 번에 요청
    WORD wPosItemNo = 0;

    // DLL 함수는 4개의 위치/속도 관련 데이터와 PT Item No.를 반환합니다.
    int nRtn = FAS_GetMotionStatus(bdID, &cmdPos, &actPos, &posErr, &actVel, &wPosItemNo);

    if (nRtn != FMM_OK) {
        // 통신 에러 발생 시, 0을 반환하거나 GUI에 에러 상태를 표시하도록 처리
        cmdPos = actPos = actVel = posErr = 0;
        return false;
    }

    // RPM 계산 (GUI 요구사항)
    // 참고: FAS_GetActualVel (Act Vel)은 [pps] 단위이므로, RPM으로 변환하려면
    // RPM = (Act Vel * 60) / (PULSE_PER_MM * LEAD_MM_PER_REVOLUTION) 공식이 필요하나,
    // 현재는 [pps]만 반환하고, GUI에서 처리하거나 별도 함수로 분리하겠습니다.

    return true;
}