#include <gl/glew.h>
#include <gl/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

#include "tinygltf-release/stb_image.h"
#include "model.h"

// --- 셰이더 소스 ---
#include "shader.h"

// --- 상태머신 ---
#include "statemachine.h"
#include "cube.h"
#include "mouse_func.h"

#define PI 3.1415f

GLuint cubeShaderProgram = 0;

// --- 윈도우 크기 ---
int windowWidth = 800;
int windowHeight = 600;

InputData inputData;
Context player_statemachine;

float lastFrameTime = 0.0f;

// --- 플레이어 및 카메라 위치 ---
glm::vec3 playerPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
float playerRotationY = 0.0f; // 플레이어 Y축 회전 각도

// --- 렌더링 루프 ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // 1. 텍스처 유닛 활성화 및 바인딩 (매우 중요)
    glActiveTexture(GL_TEXTURE0); // 0번 유닛 사용
    glBindTexture(GL_TEXTURE_2D, textureID); // 우리가 로드한 textureID 바인딩

    


    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(cameraPos, playerPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);
    
    // 플레이어 회전 적용
    model = glm::rotate(model, playerRotationY, glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "finalBonesMatrices"), 100, GL_FALSE, glm::value_ptr(finalBoneMatrices[0]));

    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    // --- 큐브 렌더링 (인스턴스드: 1회 draw call) ---
    DrawAllCubes(cubeShaderProgram, proj, view);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
switch(key) {
    case 27: // ESC 키
        glutLeaveMainLoop();
        break;
    case 'a':
    case 'A':
        inputData.isLeftDown = true;
        break;
        case 'd':
        case 'D':
            inputData.isRightDown = true;
            break;
        case 'w':
        case 'W':
            inputData.isUpDown = true;
            break;
        case 's':
        case 'S':
            inputData.isDownDown = true;
            break;
        case ' ':
            inputData.isSpaceDown = true;
            break;
        default:
            break;
	}
}

void keyboardUp(unsigned char key, int x, int y) {
    switch(key) {
        case 'a':
        case 'A':
            inputData.isLeftDown = false;
            break;
        case 'd':
        case 'D':
            inputData.isRightDown = false;
            break;
        case 'w':
        case 'W':
            inputData.isUpDown = false;
            break;
        case 's':
        case 'S':
            inputData.isDownDown = false;
            break;
        case ' ':
            inputData.isSpaceDown = false;
            break;
        default:
            break;
	}
    
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            inputData.isMouseLeftDown = true;
        }
        else if (state == GLUT_UP) {
            inputData.isMouseLeftDown = false;
        }
    }
}

void gamelogic() {
	// 1. 애니메이션 시간 업데이트
	// 2. bone matrix uniform 업데이트
	// 3. statemachine 업데이트
	
    // 1. 현재 시간 및 상대 시간 계산
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float relativeTime = currentTime - animationTime;

    aiAnimation* currentAnim = scene->mAnimations[animationIndex];
    float ticksPerSecond = (float)currentAnim->mTicksPerSecond != 0 ? (float)currentAnim->mTicksPerSecond : 25.0f;
    float timeInTicks = relativeTime * ticksPerSecond;

	float deltaTime = currentTime - lastFrameTime;

    // 카메라 회전 처리
    updateCameraRotation(deltaTime);

    // 팔방 이동에 따른 플레이어 회전
    playerRotationY = player_statemachine.calculatePlayerRotation(playerPos, cameraPos, playerRotationY);

    float animTime;

    // 2. 애니메이션 종료 감지 (현재 시간이 길이를 초과했는가?)
    /*if (timeInTicks >= (float)currentAnim->mDuration) {

        if (4 < animationIndex) {
            // 인덱스가 3보다 크면 다음 애니메이션으로 전환
            animationIndex = (animationIndex + 1) % scene->mNumAnimations;
            cout << "Auto-switched to next animation: " << animationIndex << endl;
        }
        else if (animationIndex == 3) {
            // 인덱스가 3일 때는 1으로 되돌아가기
            animationIndex = 1;
            cout << "Auto-switched back to animation: " << animationIndex << endl;
        }
        else {
            // 인덱스가 3 이하일 때는 기존처럼 반복 재생하고 싶다면?
            // (이 처리를 안 하면 3 이하일 때도 멈추거나 다음으로 넘어가지 않음)
            // 반복 재생을 원치 않고 멈추게 하려면 animTime을 mDuration으로 고정하세요.
        }

        // 애니메이션이 바뀌었거나 새로 시작해야 하므로 시작 시간을 현재로 리셋
        animationTime = currentTime;
        relativeTime = 0.0f;
        timeInTicks = 0.0f;

        // 새로 바뀐 애니메이션 정보 다시 가져오기
        currentAnim = scene->mAnimations[animationIndex];
    }

    // 3. 최종 animTime 결정
    // 인덱스가 4 이하인 경우 루프(반복)를 돌리고 싶다면 fmod를 쓰고,
    // 아니면 그냥 진행합니다.
    if (animationIndex <= 4) {
        animTime = fmod(timeInTicks, (float)currentAnim->mDuration);
    }
    else {
        animTime = timeInTicks;
    }*/
	player_statemachine.update(&animTime, timeInTicks, deltaTime);

    ReadNodeHierarchy(animTime, scene->mRootNode, glm::mat4(1.0f));

	lastFrameTime = currentTime;

	glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("GLB Bone Animation");

    glewInit();
    glEnable(GL_DEPTH_TEST);

    InitShaders();
    InitCubeShader();
    InitModel();

    // 바닥 큐브 생성 (생성자에서 자동으로 인스턴스 목록에 등록됨)
    Cube groundCube(glm::vec3(-10.0f, -2.0f, -10.0f), glm::vec3(20.0f, 0.0f, 20.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 여기에 추가 발판 큐브를 원하는 만큼 생성 가능:
    // Cube platform1(glm::vec3(...), glm::vec3(...), glm::vec3(...));
    // Cube platform2(glm::vec3(...), glm::vec3(...), glm::vec3(...));

    // 모든 큐브 생성 후, 공유 메시 + 인스턴스 버퍼 초기화
    InitCubeMesh();

    // 텍스처 로딩
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    int w, h, nrChannels;
    unsigned char* data = stbi_load("UVMAP.PNG", &w, &h, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // 데이터 정렬 문제를 방지 (가로 크기가 4배수가 아닐 때 필수)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 텍스처 파라미터 설정
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Texture loaded successfully. Channels: " << nrChannels << std::endl;
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

	// state machine 초기화
	player_statemachine.setInputData(&inputData);
	player_statemachine.init(std::make_unique<IdleState>());
    lastFrameTime = animationTime;

	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);

	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouseMotion);
	glutMotionFunc(mouseMotion);  // 드래그 시에도 동일하게 처리

    // 마우스를 윈도우 중앙으로 초기화
    centerMouse();

    glutDisplayFunc(display);
    glutIdleFunc(gamelogic);
    glutMainLoop();
    return 0;
}