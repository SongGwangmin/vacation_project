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
    if (!gNodes.empty() && !gIdleAnim.samplers.empty())
    {
        float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

        EvaluateIdle(gIdleAnim, time, gNodes);

        for (auto& n : gNodes)
            UpdateLocal(n);

        for (int r : gRootNodes)
            UpdateGlobal(r, gNodes);

        if (!gSkin.joints.empty())
        {
            BuildJointPalette(gSkin, gNodes, gJointMatrices);

            if (!gJointMatrices.empty())
            {
                glUniformMatrix4fv(
                    uJoints,
                    (GLsizei)gJointMatrices.size(),
                    GL_FALSE,
                    glm::value_ptr(gJointMatrices[0])
                );
            }
        }
    }

    /* ---- Draw ---- */
    if (gMesh.vao != 0) {
        glBindVertexArray(gMesh.vao);
        glDrawElements(
            GL_TRIANGLES,
            gMesh.indexCount,
            GL_UNSIGNED_INT,
            0
        );
        glBindVertexArray(0);
    }

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
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    gProgram = LoadShaderProgram();

    if (gProgram == 0) {
        std::cerr << "Failed to load shaders!\n";
        exit(1);
    }

    uMVP = glGetUniformLocation(gProgram, "uMVP");
    uJoints = glGetUniformLocation(gProgram, "uJoints");
}

/* =========================
   glTF Load
   ========================= */

void LoadGLTF(const char* path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool ok = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        std::cout << "GLB warning: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "GLB error: " << err << std::endl;
    }

    if (!ok) {
        std::cerr << "Failed to load GLB\n";
        exit(1);
    }

    std::cout << "Successfully loaded: " << path << std::endl;
    std::cout << "Nodes: " << model.nodes.size() << std::endl;
    std::cout << "Meshes: " << model.meshes.size() << std::endl;
    std::cout << "Skins: " << model.skins.size() << std::endl;
    std::cout << "Animations: " << model.animations.size() << std::endl;

    // ---- Load Nodes ----
    gNodes.resize(model.nodes.size());
    for (size_t i = 0; i < model.nodes.size(); i++)
    {
        const auto& gNode = model.nodes[i];
        Node& node = gNodes[i];

        // Load transform
        if (gNode.translation.size() == 3) {
            node.translation = glm::vec3(
                static_cast<float>(gNode.translation[0]),
                static_cast<float>(gNode.translation[1]),
                static_cast<float>(gNode.translation[2])
            );
        }

        if (gNode.rotation.size() == 4) {
            node.rotation = glm::quat(
                static_cast<float>(gNode.rotation[3]),  // w
                static_cast<float>(gNode.rotation[0]),  // x
                static_cast<float>(gNode.rotation[1]),  // y
                static_cast<float>(gNode.rotation[2])   // z
            );
        }

        if (gNode.scale.size() == 3) {
            node.scale = glm::vec3(
                static_cast<float>(gNode.scale[0]),
                static_cast<float>(gNode.scale[1]),
                static_cast<float>(gNode.scale[2])
            );
        }

        // Set up parent-child relationships
        node.children = gNode.children;
        for (int childIdx : gNode.children) {
            gNodes[childIdx].parent = static_cast<int>(i);
        }
    }

    // Find root nodes
    for (size_t i = 0; i < gNodes.size(); i++) {
        if (gNodes[i].parent == -1) {
            gRootNodes.push_back(static_cast<int>(i));
        }
    }

    std::cout << "Root nodes: " << gRootNodes.size() << std::endl;

    // ---- Load Skin ----
    if (!model.skins.empty())
    {
        const auto& skin = model.skins[0];
        gSkin.joints = skin.joints;

        if (skin.inverseBindMatrices >= 0) {
            gSkin.inverseBind = ReadMat4Accessor(
                model,
                model.accessors[skin.inverseBindMatrices]
            );
        }

        std::cout << "Joints: " << gSkin.joints.size() << std::endl;
    }

    // ---- Load Animation ----
    if (!model.animations.empty())
    {
        gIdleAnim = LoadIdleAnimation(model);
        std::cout << "Animation loaded: " << gIdleAnim.name 
                  << " (duration: " << gIdleAnim.duration << "s)" << std::endl;
    }

    // ---- Load Mesh ----
    if (!model.meshes.empty() && !model.meshes[0].primitives.empty())
    {
        const auto& mesh = model.meshes[0];
        const auto& primitive = mesh.primitives[0];

        std::cout << "Loading mesh geometry..." << std::endl;

        // Get attribute accessors
        auto posIt = primitive.attributes.find("POSITION");
        auto normIt = primitive.attributes.find("NORMAL");
        auto jointsIt = primitive.attributes.find("JOINTS_0");
        auto weightsIt = primitive.attributes.find("WEIGHTS_0");
        auto uvIt = primitive.attributes.find("TEXCOORD_0");

        if (posIt == primitive.attributes.end()) {
            std::cerr << "No POSITION attribute!\n";
            return;
        }

        // Read vertex data
        std::vector<glm::vec3> positions = ReadVec3Accessor(model, model.accessors[posIt->second]);
        std::vector<glm::vec3> normals;
        std::vector<glm::vec4> joints;
        std::vector<glm::vec4> weights;
        std::vector<glm::vec2> uvs;

        if (normIt != primitive.attributes.end())
            normals = ReadVec3Accessor(model, model.accessors[normIt->second]);
        
        if (jointsIt != primitive.attributes.end())
            joints = ReadVec4Accessor(model, model.accessors[jointsIt->second]);
        
        if (weightsIt != primitive.attributes.end())
            weights = ReadVec4Accessor(model, model.accessors[weightsIt->second]);

        std::cout << "Vertices: " << positions.size() << std::endl;
        std::cout << "Has joints: " << (joints.empty() ? "No" : "Yes") << std::endl;

        // Read indices
        std::vector<unsigned int> indices;
        if (primitive.indices >= 0) {
            const auto& accessor = model.accessors[primitive.indices];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];
            const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

            for (size_t i = 0; i < accessor.count; i++) {
                unsigned int index = 0;
                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    index = *reinterpret_cast<const unsigned short*>(dataPtr + i * sizeof(unsigned short));
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    index = *reinterpret_cast<const unsigned int*>(dataPtr + i * sizeof(unsigned int));
                }
                indices.push_back(index);
            }
        }

        gMesh.indexCount = static_cast<int>(indices.size());
        std::cout << "Indices: " << gMesh.indexCount << std::endl;

        // Create VAO/VBO/EBO
        glGenVertexArrays(1, &gMesh.vao);
        glGenBuffers(1, &gMesh.vbo);
        glGenBuffers(1, &gMesh.ebo);

        glBindVertexArray(gMesh.vao);

        // Interleave vertex data
        struct Vertex {
            glm::vec3 pos;
            glm::vec3 norm;
            glm::uvec4 joints;
            glm::vec4 weights;
            glm::vec2 uv;
        };

        std::vector<Vertex> vertices(positions.size());
        for (size_t i = 0; i < positions.size(); i++) {
            vertices[i].pos = positions[i];
            vertices[i].norm = normals.empty() ? glm::vec3(0, 1, 0) : normals[i];
            vertices[i].joints = joints.empty() ? glm::uvec4(0) : glm::uvec4(joints[i]);
            vertices[i].weights = weights.empty() ? glm::vec4(1, 0, 0, 0) : weights[i];
            vertices[i].uv = glm::vec2(0);
        }

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, gMesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gMesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Set vertex attributes
        // POSITION (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

        // NORMAL (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, norm));

        // JOINTS_0 (location = 2)
        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(2, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, joints));

        // WEIGHTS_0 (location = 3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

        // TEXCOORD_0 (location = 4)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

        glBindVertexArray(0);

        std::cout << "Mesh uploaded to GPU successfully!" << std::endl;
    }
}

/* =========================
   main
   ========================= */

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("glTF Idle Animation");

    InitGL();
    LoadGLTF("peto.glb");  // 또는 전체 경로

    glutDisplayFunc(Display);
    glutIdleFunc(Idle);

    glutMainLoop();
    return 0;
}