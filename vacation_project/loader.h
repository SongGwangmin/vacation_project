#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/quaternion.hpp>
#include <gl/glm/gtc/type_ptr.hpp>

#include <tinygltf-release/tiny_gltf.h>

/* =========================
   Mesh
   ========================= */

struct Mesh
{
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    int indexCount = 0;
};

/* =========================
   Node (Bone)
   ========================= */

struct Node
{
    int parent = -1;
    std::vector<int> children;

    glm::vec3 translation = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 localMatrix = glm::mat4(1.0f);
    glm::mat4 globalMatrix = glm::mat4(1.0f);
};

/* =========================
   Skin
   ========================= */

struct Skin
{
    std::vector<int> joints;
    std::vector<glm::mat4> inverseBind;
};

/* =========================
   Animation
   ========================= */

struct AnimSampler
{
    std::vector<float> times;
    std::vector<glm::vec4> values;
};

struct AnimChannel
{
    int sampler;
    int node;
    enum Path { T, R, S } path;
};

struct Animation
{
    std::string name;
    float duration = 0.0f;
    std::vector<AnimSampler> samplers;
    std::vector<AnimChannel> channels;
};

/* =========================
   Accessor Readers
   ========================= */

std::vector<float> ReadFloatAccessor(
    const tinygltf::Model&,
    const tinygltf::Accessor&);

std::vector<glm::vec3> ReadVec3Accessor(
    const tinygltf::Model&,
    const tinygltf::Accessor&);

std::vector<glm::vec4> ReadVec4Accessor(
    const tinygltf::Model&,
    const tinygltf::Accessor&);

std::vector<glm::mat4> ReadMat4Accessor(
    const tinygltf::Model&,
    const tinygltf::Accessor&);

/* =========================
   Animation Helpers
   ========================= */

Animation LoadIdleAnimation(const tinygltf::Model& model);

void EvaluateIdle(
    const Animation& anim,
    float time,
    std::vector<Node>& nodes);

void UpdateLocal(Node& n);
void UpdateGlobal(int idx, std::vector<Node>& nodes);

void BuildJointPalette(
    const Skin& skin,
    const std::vector<Node>& nodes,
    std::vector<glm::mat4>& out);

/* =========================
   Shader Utilities
   ========================= */

GLuint LoadShaderProgram();
