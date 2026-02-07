#include "cube.h"

// 전역 공유 메시 및 인스턴스 데이터 정의
GLuint g_cubeVAO = 0;
GLuint g_cubeVBO = 0;
GLuint g_cubeEBO = 0;
GLuint g_cubeInstanceVBO = 0;
std::vector<CubeInstanceData> g_cubeInstances;
