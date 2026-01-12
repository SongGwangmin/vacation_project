#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

#include <iostream>
#include <fstream>
#include <sstream>

#include "tinygltf-release/tiny_gltf.h"
#include "tinygltf-release/stb_image.h"



struct StaticMesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int indexCount;
};

std::vector<float> ReadFloatAccessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor);

std::vector<unsigned int> ReadIndexAccessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor);

StaticMesh LoadStaticMesh(const char* path);

std::string LoadTextFile(const char* path);

GLuint LoadTexture(const char* pngPath);