#include "mouse_func.h"

// --- 마우스 이동량 누적 변수 정의 ---
int accumulatedDeltaX = 0;
int accumulatedDeltaY = 0;

void centerMouse() {
    glutWarpPointer(windowWidth / 2, windowHeight / 2);
}

void mouseMotion(int x, int y) {
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;
    
    accumulatedDeltaX += x - centerX;
    accumulatedDeltaY += -(y - centerY); // Y축 반전 처리
    
    centerMouse();
}

void updateCameraRotation(float deltaTime) {
    if (accumulatedDeltaX == 0 && accumulatedDeltaY == 0) {
        return;
    }
    
    // playerPos를 기준으로 cameraPos의 상대 위치 계산
    glm::vec3 offset = cameraPos - playerPos;
    
    // deltaX: Y축 기준 회전 (ZX 평면 회전)
    if (accumulatedDeltaX != 0) {
        float angleX = (float)accumulatedDeltaX * MOUSE_SENSITIVITY_X;
        
        float cosA = cos(angleX);
        float sinA = sin(angleX);
        float newX = offset.x * cosA - offset.z * sinA;
        float newZ = offset.x * sinA + offset.z * cosA;
        
        offset = glm::vec3(newX, offset.y, newZ);
    }
    
    // deltaY: 수평축 기준 회전 (YZ 평면 회전, 카메라 상하 이동)
    if (accumulatedDeltaY != 0) {
        float angleY = (float)accumulatedDeltaY * MOUSE_SENSITIVITY_Y;
        
        // 현재 offset의 수평 거리 계산
        float horizontalDist = sqrt(offset.x * offset.x + offset.z * offset.z);
        
        // 수평축 기준 회전 (YZ 평면에서 Y와 수평거리를 회전)
        float cosB = cos(angleY);
        float sinB = sin(angleY);
        float newY = offset.y * cosB - horizontalDist * sinB;
        float newHorizontalDist = offset.y * sinB + horizontalDist * cosB;
        
        // 수평 거리 비율 유지하면서 X, Z 재계산
        if (horizontalDist > 0.001f) {
            float ratio = newHorizontalDist / horizontalDist;
            float newOffsetX = offset.x * ratio;
            float newOffsetZ = offset.z * ratio;
            
            // X 또는 Z의 부호가 반전되면 반영하지 않음
            if (!(offset.x * newOffsetX < 0 || offset.z * newOffsetZ < 0)) {
                offset = glm::vec3(newOffsetX, newY, newOffsetZ);
            }
        }
    }
    
    cameraPos = playerPos + offset;
    
    // 누적값 초기화
    accumulatedDeltaX = 0;
    accumulatedDeltaY = 0;
}
