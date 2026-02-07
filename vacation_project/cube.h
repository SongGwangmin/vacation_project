#pragma once

#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <limits>

// 인스턴스별 데이터 (GPU에 업로드될 구조체)
struct CubeInstanceData {
    glm::vec3 offset; // 중심 위치
    glm::vec3 scale;  // 크기
    glm::vec3 color;  // 색상
};

// 전역 공유 메시 및 인스턴스 데이터 (extern 선언)
extern GLuint g_cubeVAO;
extern GLuint g_cubeVBO;
extern GLuint g_cubeEBO;
extern GLuint g_cubeInstanceVBO;
extern std::vector<CubeInstanceData> g_cubeInstances;

// Ray-AABB 충돌 검사 (Slab Method)
// rayOrigin: 레이 시작점
// rayDir: 레이 방향 (정규화 필요 없음)
// minPos, maxPos: AABB 경계
// tMin: 충돌 시작 지점 (out)
// 반환: 충돌 여부
inline bool RayAABBIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                              const glm::vec3& minPos, const glm::vec3& maxPos,
                              float& tMin) {
    float tmax = std::numeric_limits<float>::max();
    tMin = 0.0f;
    
    for (int i = 0; i < 3; ++i) {
        if (std::abs(rayDir[i]) < 1e-6f) {
            // 레이가 이 축과 평행
            if (rayOrigin[i] < minPos[i] || rayOrigin[i] > maxPos[i]) {
                return false; // 슬랩 바깥에 있음
            }
        } else {
            float invD = 1.0f / rayDir[i];
            float t1 = (minPos[i] - rayOrigin[i]) * invD;
            float t2 = (maxPos[i] - rayOrigin[i]) * invD;
            
            if (t1 > t2) std::swap(t1, t2);
            
            tMin = std::max(tMin, t1);
            tmax = std::min(tmax, t2);
            
            if (tMin > tmax) {
                return false; // 교차 구간 없음
            }
        }
    }
    
    return tmax >= 0.0f; // 충돌이 레이 앞쪽에 있음
}

// 모든 큐브와 레이 충돌 검사 후, 가장 가까운 충돌점으로 카메라 위치 조정
// playerPos: 레이 시작점 (플레이어 위치)
// desiredCameraPos: 원하는 카메라 위치
// minDistance: 충돌점에서 얘간 떨어진 오프셋
// 반환: 조정된 카메라 위치
inline glm::vec3 GetAdjustedCameraPos(const glm::vec3& playerPos,
                                       const glm::vec3& desiredCameraPos,
                                       float minDistance = 0.0f) {
    // 플레이어 눈높이에서 레이 시작 (바닥 큐브와의 경계 문제 방지)
    glm::vec3 rayOrigin = playerPos + glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 rayDir = desiredCameraPos - rayOrigin;
    float maxT = glm::length(rayDir);
    
    if (maxT < 1e-6f) {
        return desiredCameraPos; // 플레이어와 카메라가 같은 위치
    }
    
    glm::vec3 rayDirNorm = rayDir / maxT; // 정규화
    float closestT = maxT; // 가장 가까운 충돌 지점
    bool hasCollision = false;
    
    for (const auto& cube : g_cubeInstances) {
        // 인스턴스 데이터에서 AABB 계산
        glm::vec3 halfScale = cube.scale * 0.5f;
        glm::vec3 minPos = cube.offset - halfScale;
        glm::vec3 maxPos = cube.offset + halfScale;
        
        float tHit;
        if (RayAABBIntersect(rayOrigin, rayDirNorm, minPos, maxPos, tHit)) {
            // 충돌이 레이 시작점과 카메라 사이에 있는지 확인
            if (tHit > 0.0f && tHit < closestT) {
                closestT = tHit;
                hasCollision = true;
            }
        }
    }
    
    // 충돌이 있으면 충돌점 직전으로 카메라 이동
    if (hasCollision) {
        float adjustedT = std::max(closestT - minDistance, 0.1f);
        return rayOrigin + rayDirNorm * adjustedT;
    }
    
    return desiredCameraPos;
}

// 공유 단위 큐브 메시 생성 + 인스턴스 버퍼를 GPU에 업로드
// 모든 Cube 객체를 생성한 뒤, 렌더링 전에 1회 호출
inline void InitCubeMesh() {
    float vertices[] = {
        // 단위 큐브: -0.5 ~ 0.5
        -0.5f, -0.5f,  0.5f,  // 0
         0.5f, -0.5f,  0.5f,  // 1
         0.5f,  0.5f,  0.5f,  // 2
        -0.5f,  0.5f,  0.5f,  // 3
        -0.5f, -0.5f, -0.5f,  // 4
         0.5f, -0.5f, -0.5f,  // 5
         0.5f,  0.5f, -0.5f,  // 6
        -0.5f,  0.5f, -0.5f,  // 7
    };

    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0,  // 앞면
        5, 4, 7,  7, 6, 5,  // 뒷면
        4, 0, 3,  3, 7, 4,  // 왼쪽
        1, 5, 6,  6, 2, 1,  // 오른쪽
        3, 2, 6,  6, 7, 3,  // 윗면
        4, 5, 1,  1, 0, 4,  // 아랫면
    };

    glGenVertexArrays(1, &g_cubeVAO);
    glGenBuffers(1, &g_cubeVBO);
    glGenBuffers(1, &g_cubeEBO);
    glGenBuffers(1, &g_cubeInstanceVBO);

    glBindVertexArray(g_cubeVAO);

    // --- 메시 데이터 (모든 인스턴스가 공유) ---
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // location 0: 정점 위치 (정점마다)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- 인스턴스 데이터 ---
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 g_cubeInstances.size() * sizeof(CubeInstanceData),
                 g_cubeInstances.data(),
                 GL_STATIC_DRAW);

    // location 1: offset (인스턴스마다)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CubeInstanceData),
                          (void*)offsetof(CubeInstanceData, offset));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // location 2: scale (인스턴스마다)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CubeInstanceData),
                          (void*)offsetof(CubeInstanceData, scale));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // location 3: color (인스턴스마다)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(CubeInstanceData),
                          (void*)offsetof(CubeInstanceData, color));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

// 모든 큐브를 1회의 draw call로 렌더링
inline void DrawAllCubes(GLuint shaderProg, const glm::mat4& projection, const glm::mat4& view) {
    if (g_cubeInstances.empty()) return;

    glUseProgram(shaderProg);
    glUniformMatrix4fv(glGetUniformLocation(shaderProg, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProg, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(g_cubeVAO);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, (GLsizei)g_cubeInstances.size());
    glBindVertexArray(0);
}

class Cube {
public:
    glm::vec3 minPos;
    glm::vec3 maxPos;
    glm::vec3 color;

    Cube()
        : minPos(0.0f), maxPos(1.0f), color(1.0f) {}

    Cube(glm::vec3 minP, glm::vec3 maxP, glm::vec3 col)
        : minPos(minP), maxPos(maxP), color(col)
    {
        // 인스턴스 데이터를 전역 목록에 자동 등록
        CubeInstanceData inst;
        inst.offset = (minPos + maxPos) * 0.5f;  // 중심
        inst.scale  = maxPos - minPos;            // 크기
        inst.color  = color;
        g_cubeInstances.push_back(inst);
    }
};
