#include "loader.h"

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

StaticMesh LoadStaticMesh(const char* path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    loader.LoadBinaryFromFile(&model, &err, &warn, path);

    const tinygltf::Mesh& mesh = model.meshes[0];
    const tinygltf::Primitive& prim = mesh.primitives[0];

    // POSITION
    int posAccIndex = prim.attributes.at("POSITION");
    const tinygltf::Accessor& posAccessor = model.accessors[posAccIndex];
    std::vector<float> positions = ReadFloatAccessor(model, posAccessor);

    // INDICES
    const tinygltf::Accessor& idxAccessor =
        model.accessors[prim.indices];
    std::vector<unsigned int> indices =
        ReadIndexAccessor(model, idxAccessor);

    StaticMesh out{};
    out.indexCount = (int)indices.size();

    glGenVertexArrays(1, &out.vao);
    glGenBuffers(1, &out.vbo);
    glGenBuffers(1, &out.ebo);

    glBindVertexArray(out.vao);

    glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
    glBufferData(GL_ARRAY_BUFFER,
        positions.size() * sizeof(float),
        positions.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return out;
}
