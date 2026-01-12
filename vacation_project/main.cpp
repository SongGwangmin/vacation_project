#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>



#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>

#include "loader.h"


GLuint gProgram = 0;
GLuint gVAO = 0;
GLuint gVBO = 0;

StaticMesh gMesh;



// =======================
// 셰이더 컴파일
// =======================
GLuint CompileShader(GLenum type, const char* path)
{
    std::string srcStr = LoadTextFile(path);
    const char* src = srcStr.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error (" << path << "):\n"
            << log << std::endl;
    }
    return shader;
}

// =======================
// OpenGL 초기화
// =======================
void InitOpenGL()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to init GLEW\n";
        exit(-1);
    }

    glEnable(GL_DEPTH_TEST);

    // --- 셰이더 로드 ---
    GLuint vs = CompileShader(GL_VERTEX_SHADER, "vertex.glsl");
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, "fragment.glsl");

    gProgram = glCreateProgram();
    glAttachShader(gProgram, vs);
    glAttachShader(gProgram, fs);
    glLinkProgram(gProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    // --- 임시 삼각형 ---
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &gVAO);
    glGenBuffers(1, &gVBO);

    glBindVertexArray(gVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    gMesh = LoadStaticMesh("peto.glb");

}

// =======================
// 렌더
// =======================
void Display()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgram);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 2),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 proj = glm::perspective(
        glm::radians(60.0f),
        800.0f / 600.0f,
        0.1f,
        100.0f
    );

    glm::mat4 mvp = proj * view * model;
    GLint loc = glGetUniformLocation(gProgram, "uMVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(gMesh.vao);
    glDrawElements(GL_TRIANGLES,
        gMesh.indexCount,
        GL_UNSIGNED_INT,
        0);
    glBindVertexArray(0);

    glutSwapBuffers();
}

void Idle()
{
    glutPostRedisplay();
}

// =======================
// main
// =======================
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("GLEW + FreeGLUT (External Shader)");

    InitOpenGL();

    glutDisplayFunc(Display);
    glutIdleFunc(Idle);

    glutMainLoop();
    return 0;
}