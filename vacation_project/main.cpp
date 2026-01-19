#include <gl/glew.h>
#include <gl/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

// GLM
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include <gl/glm/gtc/quaternion.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "tinygltf-release/stb_image.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "assimp-vc143-mt.lib") // 버전에 따라 이름이 다를 수 있음

using namespace std;

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

// --- 전역 변수 ---
GLuint shaderProgram;
GLuint VAO, VBO, EBO, textureID;
unsigned int indexCount;

Assimp::Importer importer;
const aiScene* scene = nullptr;
map<string, BoneInfo> m_BoneInfoMap;
int m_BoneCounter = 0;
glm::mat4 m_GlobalInverseTransform;
vector<glm::mat4> finalBoneMatrices(100, glm::mat4(1.0f));

// --- 헬퍼 함수: Assimp 행렬을 GLM 행렬로 변환 ---
glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

// --- 애니메이션 보간 함수들 ---
unsigned int FindRotation(float animationTime, const aiNodeAnim* pNodeAnim) {
    for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
        if (animationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) return i;
    }
    return 0;
}

unsigned int FindPosition(float animationTime, const aiNodeAnim* pNodeAnim) {
    for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
        if (animationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) return i;
    }
    return 0;
}

unsigned int FindScaling(float animationTime, const aiNodeAnim* pNodeAnim) {
    for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
        if (animationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) return i;
    }
    return 0;
}

// --- 노드 계층 구조 순회 및 행렬 계산 ---
void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform) {
    string NodeName(pNode->mName.data);
    const aiAnimation* pAnimation = scene->mAnimations[0]; // 0번 애니메이션 고정
    glm::mat4 NodeTransformation = ConvertMatrixToGLMFormat(pNode->mTransformation);

    const aiNodeAnim* pNodeAnim = nullptr;
    for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
        if (string(pAnimation->mChannels[i]->mNodeName.data) == NodeName) {
            pNodeAnim = pAnimation->mChannels[i];
            break;
        }
    }

    if (pNodeAnim) {
        // 1. Rotation 보간
        unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
        unsigned int NextRotationIndex = (RotationIndex + 1);
        float deltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
        float factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / deltaTime;
        aiQuaternion OutR;
        aiQuaternion::Interpolate(OutR, pNodeAnim->mRotationKeys[RotationIndex].mValue, pNodeAnim->mRotationKeys[NextRotationIndex].mValue, factor);
        glm::quat q(OutR.w, OutR.x, OutR.y, OutR.z);
        glm::mat4 rotationM = glm::mat4_cast(q);

        // 2. Position 보간
        unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
        unsigned int NextPositionIndex = (PositionIndex + 1);
        deltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
        factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / deltaTime;
        aiVector3D OutP;
        OutP = pNodeAnim->mPositionKeys[PositionIndex].mValue + (pNodeAnim->mPositionKeys[NextPositionIndex].mValue - pNodeAnim->mPositionKeys[PositionIndex].mValue) * factor;
        glm::mat4 translationM = glm::translate(glm::mat4(1.0f), glm::vec3(OutP.x, OutP.y, OutP.z));

        NodeTransformation = translationM * rotationM;
    }

    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneInfoMap.find(NodeName) != m_BoneInfoMap.end()) {
        int BoneIndex = m_BoneInfoMap[NodeName].id;
        finalBoneMatrices[BoneIndex] = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfoMap[NodeName].offset;
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHierarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}

// --- 초기화 함수 ---
void InitModel() {
    scene = importer.ReadFile("peto.glb", aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_FlipUVs);
    if (!scene) { cerr << "Model Load Failed!" << endl; exit(1); }

    m_GlobalInverseTransform = glm::inverse(ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation));

    vector<Vertex> vertices;
    vector<unsigned int> indices;
    aiMesh* mesh = scene->mMeshes[0]; // 첫 번째 메시만 로드

    // 1. 뼈 정보 미리 맵핑
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        string boneName = mesh->mBones[i]->mName.C_Str();
        int boneID = -1;
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
            boneID = m_BoneCounter++;
            BoneInfo bi; bi.id = boneID;
            bi.offset = ConvertMatrixToGLMFormat(mesh->mBones[i]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = bi;
        }
    }

    // 2. 정점 데이터 추출
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if (mesh->mTextureCoords[0])
            v.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        vertices.push_back(v);
    }

    // 3. 뼈 가중치 데이터 추출
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        int boneID = m_BoneInfoMap[mesh->mBones[i]->mName.C_Str()].id;
        auto weights = mesh->mBones[i]->mWeights;
        for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
            int vertexID = weights[j].mVertexId;
            float weight = weights[j].mWeight;
            for (int k = 0; k < 4; k++) {
                if (vertices[vertexID].BoneIDs[k] == -1) {
                    vertices[vertexID].BoneIDs[k] = boneID;
                    vertices[vertexID].Weights[k] = weight;
                    break;
                }
            }
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        for (unsigned int j = 0; j < 3; j++) indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
    indexCount = indices.size();

    // 4. OpenGL 버퍼 설정
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); // UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2); // BoneIDs (int 타입은 IPointer)
    glVertexAttribIPointer(2, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));
    glEnableVertexAttribArray(3); // Weights
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));
}

// --- 셰이더 소스 ---
/*const char* vsSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in ivec4 aBoneIds;
layout (location = 3) in vec4 aWeights;
uniform mat4 finalBonesMatrices[100];
uniform mat4 projection, view, model;
out vec2 TexCoord;
void main() {
    vec4 totalPos = vec4(0.0);
    for(int i=0; i<4; i++) {
        if(aBoneIds[i] == -1) continue;
        totalPos += (finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1.0)) * aWeights[i];
    }
    gl_Position = projection * view * model * totalPos;
    TexCoord = aTex;
})";

const char* fsSource = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D tex;
void main() { FragColor = texture(tex, TexCoord); })";*/

std::string ReadShaderFile(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void InitShaders() {
    // 1. 파일에서 소스 읽기
    std::string vsCode = ReadShaderFile("vertex.glsl");
    std::string fsCode = ReadShaderFile("fragment.glsl");

    const char* vsSource = vsCode.c_str();
    const char* fsSource = fsCode.c_str();

    GLint success;
    char infoLog[512];

    // 2. Vertex Shader 컴파일
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, NULL);
    glCompileShader(vs);

    // 컴파일 에러 체크
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 3. Fragment Shader 컴파일
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, NULL);
    glCompileShader(fs);

    // 컴파일 에러 체크
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 4. Shader Program 링크
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // 링크 에러 체크
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(shaderProgram);
}

// --- 렌더링 루프 ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float ticksPerSecond = (float)scene->mAnimations[0]->mTicksPerSecond != 0 ? (float)scene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float timeInTicks = time * ticksPerSecond;
    float animTime = fmod(timeInTicks, (float)scene->mAnimations[0]->mDuration);

    ReadNodeHierarchy(animTime, scene->mRootNode, glm::mat4(1.0f));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 2, 5), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "finalBonesMatrices"), 100, GL_FALSE, glm::value_ptr(finalBoneMatrices[0]));

    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("GLB Bone Animation");

    glewInit();
    glEnable(GL_DEPTH_TEST);

    InitShaders();
    InitModel();

    // 텍스처 로딩
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    int w, h, c;
    unsigned char* data = stbi_load("UVMAP.png", &w, &h, &c, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}