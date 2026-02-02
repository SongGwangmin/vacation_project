#include <gl/glew.h>
#include <gl/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

#include "model.h"

// --- 셰이더 소스 ---
#include "shader.h"

// --- 렌더링 루프 ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // 1. 텍스처 유닛 활성화 및 바인딩 (매우 중요)
    glActiveTexture(GL_TEXTURE0); // 0번 유닛 사용
    glBindTexture(GL_TEXTURE_2D, textureID); // 우리가 로드한 textureID 바인딩

    // 1. 현재 시간 및 상대 시간 계산
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float relativeTime = currentTime - animationTime;

    aiAnimation* currentAnim = scene->mAnimations[animationIndex];
    float ticksPerSecond = (float)currentAnim->mTicksPerSecond != 0 ? (float)currentAnim->mTicksPerSecond : 25.0f;
    float timeInTicks = relativeTime * ticksPerSecond;

    float animTime;

    // 2. 애니메이션 종료 감지 (현재 시간이 길이를 초과했는가?)
    if (timeInTicks >= (float)currentAnim->mDuration) {

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
    }


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

void keyboard(unsigned char key, int x, int y) {
    if (key == 'n') {
        animationIndex = (animationIndex + 1) % scene->mNumAnimations;

		animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

        cout << "Switched to animation index: " << animationIndex << endl;



    }
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

	glutKeyboardFunc(keyboard);

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}