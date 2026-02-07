#pragma once

#include "cube.h"  // glew.h를 먼저 포함하기 위해 cube.h를 먼저 include
#include <gl/freeglut.h>
#include <gl/glm/glm.hpp>
#include <cmath>

// --- 마우스 감도 ---
#define MOUSE_SENSITIVITY_X 0.005f
#define MOUSE_SENSITIVITY_Y 0.0025f

// --- 마우스 이동량 누적 ---
extern int accumulatedDeltaX;
extern int accumulatedDeltaY;

// --- 윈도우 크기 (extern 참조) ---
extern int windowWidth;
extern int windowHeight;

// --- 카메라 및 플레이어 위치 (extern 참조) ---
extern glm::vec3 playerPos;
extern glm::vec3 cameraPos;
extern glm::vec3 desiredCameraOffset;  // 목표 카메라 오프셋 (충돌 전 원래 거리 유지용)

// --- 함수 선언 ---
void centerMouse();
void mouseMotion(int x, int y);
void updateCameraRotation(float deltaTime);
