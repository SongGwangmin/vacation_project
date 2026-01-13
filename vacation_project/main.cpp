#include <iostream>
#include <vector>

#include <gl/glew.h>
#include <gl/freeglut.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <tinygltf-release/tiny_gltf.h>

#include "loader.h"

/* =========================
   Globals
   ========================= */

GLuint gProgram = 0;
Mesh gMesh;

std::vector<Node> gNodes;
std::vector<int> gRootNodes;

Skin gSkin;
Animation gIdleAnim;
std::vector<glm::mat4> gJointMatrices;

GLint uMVP = -1;
GLint uJoints = -1;

/* =========================
   Utility
   ========================= */

//GLuint LoadShaderProgram(); // 네가 이미 만든다고 했으니 선언만

/* =========================
   Display
   ========================= */

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(gProgram);

    /* ---- Camera ---- */
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 3, 5),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 proj = glm::perspective(
        glm::radians(60.0f),
        800.0f / 600.0f,
        0.1f,
        100.0f
    );

    glm::mat4 mvp = proj * view * model;
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));

    /* ---- Animation ---- */
    float time =
        glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    EvaluateIdle(gIdleAnim, time, gNodes);

    for (auto& n : gNodes)
        UpdateLocal(n);

    for (int r : gRootNodes)
        UpdateGlobal(r, gNodes);

    BuildJointPalette(gSkin, gNodes, gJointMatrices);

    glUniformMatrix4fv(
        uJoints,
        (GLsizei)gJointMatrices.size(),
        GL_FALSE,
        glm::value_ptr(gJointMatrices[0])
    );

    /* ---- Draw ---- */
    glBindVertexArray(gMesh.vao);
    glDrawElements(
        GL_TRIANGLES,
        gMesh.indexCount,
        GL_UNSIGNED_INT,
        0
    );
    glBindVertexArray(0);

    glutSwapBuffers();
}

/* =========================
   Idle Loop
   ========================= */

void Idle()
{
    glutPostRedisplay();
}

/* =========================
   GL Init
   ========================= */

void InitGL()
{
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    gProgram = LoadShaderProgram();

    uMVP = glGetUniformLocation(gProgram, "uMVP");
    uJoints = glGetUniformLocation(gProgram, "uJoints");
}

/* =========================
   glTF Load (최소)
   ========================= */

void LoadGLTF(const char* path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool ok = loader.LoadBinaryFromFile(
        &model, &err, &warn, path);

    if (!ok)
    {
        std::cerr << "GLB load failed\n";
        exit(1);
    }

    /* ---- Nodes ---- */
    gNodes.resize(model.nodes.size());

    for (int i = 0; i < model.nodes.size(); i++)
    {
        const auto& n = model.nodes[i];
        Node& dst = gNodes[i];

        if (n.translation.size() == 3)
            dst.translation = glm::vec3(
                n.translation[0],
                n.translation[1],
                n.translation[2]);

        if (n.rotation.size() == 4)
            dst.rotation = glm::quat(
                n.rotation[3],
                n.rotation[0],
                n.rotation[1],
                n.rotation[2]);

        if (n.scale.size() == 3)
            dst.scale = glm::vec3(
                n.scale[0],
                n.scale[1],
                n.scale[2]);

        for (int c : n.children)
        {
            dst.children.push_back(c);
            gNodes[c].parent = i;
        }
    }

    for (int i = 0; i < gNodes.size(); i++)
        if (gNodes[i].parent == -1)
            gRootNodes.push_back(i);

    /* ---- Skin ---- */
    const auto& skin = model.skins[0];
    gSkin.joints = skin.joints;

    auto ibm =
        ReadVec4Accessor(model,
            model.accessors[skin.inverseBindMatrices]);

    gSkin.inverseBind.resize(gSkin.joints.size());

    for (int i = 0; i < gSkin.joints.size(); i++)
        gSkin.inverseBind[i] =
        glm::make_mat4(&ibm[i * 4].x);

    /* ---- Animation ---- */
    gIdleAnim = LoadIdleAnimation(model);

    /* ---- Mesh ---- */
    // ⚠️ 정적 메시 로딩은
    // 네가 이미 성공시킨 코드 그대로 사용
}

/* =========================
   main
   ========================= */

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(
        GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("glTF Idle Animation");

    InitGL();
    LoadGLTF("peto.glb");

    glutDisplayFunc(Display);
    glutIdleFunc(Idle);

    glutMainLoop();
    return 0;
}
