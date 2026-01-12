#include "loader.h"


static std::vector<float> ReadVec3(
    const tinygltf::Model& model,
    const tinygltf::Accessor& acc)
{
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buf = model.buffers[view.buffer];
    const unsigned char* data =
        buf.data.data() + view.byteOffset + acc.byteOffset;

    std::vector<float> out(acc.count * 3);
    memcpy(out.data(), data, out.size() * sizeof(float));
    return out;
}

static std::vector<float> ReadVec2(
    const tinygltf::Model& model,
    const tinygltf::Accessor& acc)
{
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buf = model.buffers[view.buffer];
    const unsigned char* data =
        buf.data.data() + view.byteOffset + acc.byteOffset;

    std::vector<float> out(acc.count * 2);
    memcpy(out.data(), data, out.size() * sizeof(float));
    return out;
}

static std::vector<unsigned int> ReadIndices(
    const tinygltf::Model& model,
    const tinygltf::Accessor& acc)
{
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buf = model.buffers[view.buffer];
    const unsigned char* data =
        buf.data.data() + view.byteOffset + acc.byteOffset;

    std::vector<unsigned int> out(acc.count);

    if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* src = (const uint16_t*)data;
        for (size_t i = 0; i < acc.count; i++) out[i] = src[i];
    }
    else
    {
        memcpy(out.data(), data, acc.count * sizeof(uint32_t));
    }
    return out;
}



std::vector<float> ReadFloatAccessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor)
{
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[view.buffer];

    const unsigned char* data =
        buffer.data.data() + view.byteOffset + accessor.byteOffset;

    size_t count = accessor.count;
    size_t componentSize = sizeof(float);

    std::vector<float> result(count * 3);
    memcpy(result.data(), data, result.size() * componentSize);

    return result;
}

std::vector<unsigned int> ReadIndexAccessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor)
{
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[view.buffer];
    const unsigned char* data =
        buffer.data.data() + view.byteOffset + accessor.byteOffset;

    std::vector<unsigned int> indices;
    indices.resize(accessor.count);

    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
    {
        const uint16_t* src = (const uint16_t*)data;
        for (size_t i = 0; i < accessor.count; i++)
            indices[i] = src[i];
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
    {
        memcpy(indices.data(), data, accessor.count * sizeof(uint32_t));
    }

    return indices;
}

// ==========================
// GLB ¡æ StaticMesh
// ==========================
StaticMesh LoadStaticMesh(const char* path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    if (!loader.LoadBinaryFromFile(&model, &err, &warn, path))
        throw std::runtime_error("GLB load failed");

    const auto& prim = model.meshes[0].primitives[0];

    auto pos = ReadVec3(model, model.accessors[prim.attributes.at("POSITION")]);
    auto uv = ReadVec2(model, model.accessors[prim.attributes.at("TEXCOORD_0")]);
    auto idx = ReadIndices(model, model.accessors[prim.indices]);

    std::vector<float> vertices;
    for (size_t i = 0; i < pos.size() / 3; i++)
    {
        vertices.push_back(pos[i * 3 + 0]);
        vertices.push_back(pos[i * 3 + 1]);
        vertices.push_back(pos[i * 3 + 2]);
        vertices.push_back(uv[i * 2 + 0]);
        vertices.push_back(uv[i * 2 + 1]);
    }

    StaticMesh mesh{};
    mesh.indexCount = (int)idx.size();

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        idx.size() * sizeof(unsigned int),
        idx.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return mesh;
}

// ==========================
// PNG Texture Loader
// ==========================
GLuint LoadTexture(const char* path)
{
    int w, h, c;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(path, &w, &h, &c, 4);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        w, h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

// ==========================
// Text File Loader
// ==========================

std::string LoadTextFile(const char* path)
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