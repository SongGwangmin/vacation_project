#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>

// OpenGL
#include <gl/glew.h>
#include <gl/freeglut.h>

// GLM
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include <gl/glm/gtc/quaternion.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "assimp-vc143-mt.lib") // 버전에 따라 이름이 다를 수 있음

// --- 구조체 정의 ---
struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
    int BoneIDs[4] = { -1, -1, -1, -1 };
    float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct BoneInfo {
    int id;
    glm::mat4 offset;
};

// --- 전역 변수 선언 (정의는 model.cpp에) ---
extern GLuint shaderProgram;
extern GLuint VAO, VBO, EBO, textureID;
extern unsigned int indexCount;

extern int animationIndex;
extern float animationTime;

extern Assimp::Importer importer;
extern const aiScene* scene;
extern std::map<std::string, BoneInfo> m_BoneInfoMap;
extern int m_BoneCounter;
extern glm::mat4 m_GlobalInverseTransform;
extern std::vector<glm::mat4> finalBoneMatrices;

// --- 함수 선언 (정의는 model.cpp에) ---
glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from);
unsigned int FindRotation(float animationTime, const aiNodeAnim* pNodeAnim);
unsigned int FindPosition(float animationTime, const aiNodeAnim* pNodeAnim);
unsigned int FindScaling(float animationTime, const aiNodeAnim* pNodeAnim);
void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
void InitModel();
