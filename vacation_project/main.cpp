#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <gl/glew.h>
#include <gl/freeglut.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <tinygltf-release/tiny_gltf.h>

#include "loader.h"

GLuint gProgram;
Mesh gMesh;

std::vector<Node> gNodes;
std::vector<int> gRootNodes;

Skin gSkin;
Animation gIdle;
std::vector<glm::mat4> gJointMatrices;

GLint uMVP, uJoints;

static std::string LoadTextFile(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

/* =========================
   Shader compile
   ========================= */

static GLuint CompileShader(const char* path, GLenum type)
{
    std::string source = LoadTextFile(path);
    if (source.empty())
        return 0;

    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader compile error (" << path << ")\n"
            << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

/* =========================
   Program link
   ========================= */

GLuint LoadShaderProgram()
{
    GLuint vs = CompileShader("vertex.glsl", GL_VERTEX_SHADER);
    GLuint fs = CompileShader("fragment.glsl", GL_FRAGMENT_SHADER);

    if (!vs || !fs)
    {
        std::cerr << "Shader compilation failed" << std::endl;
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "Program link error\n"
            << log << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(gProgram);

    glm::mat4 mvp =
        glm::perspective(glm::radians(60.0f), 800.f / 600.f, 0.1f, 100.f) *
        glm::lookAt(glm::vec3(0, 3, 5),
            glm::vec3(0, 1, 0),
            glm::vec3(0, 1, 0));

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));

    float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    EvaluateIdle(gIdle, time, gNodes);

    for (auto& n : gNodes) UpdateLocal(n);
    for (int r : gRootNodes) UpdateGlobal(r, gNodes);

    BuildJointPalette(gSkin, gNodes, gJointMatrices);

    glUniformMatrix4fv(
        uJoints,
        (GLsizei)gJointMatrices.size(),
        GL_FALSE,
        glm::value_ptr(gJointMatrices[0]));

    glBindVertexArray(gMesh.vao);
    glDrawElements(GL_TRIANGLES, gMesh.indexCount,
        GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glutSwapBuffers();
}

void Idle() { glutPostRedisplay(); }

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("glTF Idle");

    glewInit();
    glEnable(GL_DEPTH_TEST);

    gProgram = LoadShaderProgram();
    uMVP = glGetUniformLocation(gProgram, "uMVP");
    uJoints = glGetUniformLocation(gProgram, "uJoints");

    // ⚠️ 메시 로딩은 네 기존 코드 사용

    glutDisplayFunc(Display);
    glutIdleFunc(Idle);
    glutMainLoop();
    return 0;
}
