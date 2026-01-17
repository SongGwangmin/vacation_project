#include "loader.h"
#include <cmath>

/* =========================
   Accessor Readers
   ========================= */

std::vector<float> ReadFloatAccessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor)
{
    std::vector<float> out;

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buf = model.buffers[view.buffer];

    const unsigned char* data =
        buf.data.data() + view.byteOffset + accessor.byteOffset;

    size_t count = accessor.count;
    size_t stride = accessor.ByteStride(view);
    if (stride == 0) stride = sizeof(float);

    out.resize(count);

    for (size_t i = 0; i < count; i++)
        out[i] = *reinterpret_cast<const float*>(data + i * stride);

    return out;
}

std::vector<glm::vec3> ReadVec3Accessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor)
{
    std::vector<glm::vec3> out;

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buf = model.buffers[view.buffer];

    const unsigned char* data =
        buf.data.data() + view.byteOffset + accessor.byteOffset;

    out.resize(accessor.count);

    for (size_t i = 0; i < accessor.count; i++)
    {
        const float* p =
            reinterpret_cast<const float*>(data + i * sizeof(float) * 3);
        out[i] = glm::vec3(p[0], p[1], p[2]);
    }
    return out;
}

std::vector<glm::vec4> ReadVec4Accessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor)
{
    std::vector<glm::vec4> out;

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buf = model.buffers[view.buffer];

    const unsigned char* data =
        buf.data.data() + view.byteOffset + accessor.byteOffset;

    out.resize(accessor.count);

    for (size_t i = 0; i < accessor.count; i++)
    {
        const float* p =
            reinterpret_cast<const float*>(data + i * sizeof(float) * 4);
        out[i] = glm::vec4(p[0], p[1], p[2], p[3]);
    }
    return out;
}

std::vector<glm::mat4> ReadMat4Accessor(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor)
{
    std::vector<glm::mat4> out;

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buf = model.buffers[view.buffer];

    const unsigned char* data =
        buf.data.data() + view.byteOffset + accessor.byteOffset;

    out.resize(accessor.count);

    for (size_t i = 0; i < accessor.count; i++)
    {
        const float* p =
            reinterpret_cast<const float*>(data + i * sizeof(float) * 16);
        out[i] = glm::make_mat4(p);
    }
    return out;
}

/* =========================
   Animation
   ========================= */

Animation LoadIdleAnimation(const tinygltf::Model& model)
{
    Animation anim;

    if (model.animations.empty())
        return anim;

    const auto& src = model.animations[0];
    anim.name = src.name;

    for (const auto& s : src.samplers)
    {
        AnimSampler sp;
        sp.times = ReadFloatAccessor(model, model.accessors[s.input]);
        sp.values = ReadVec4Accessor(model, model.accessors[s.output]);
        anim.samplers.push_back(sp);
    }

    for (const auto& c : src.channels)
    {
        AnimChannel ch;
        ch.sampler = c.sampler;
        ch.node = c.target_node;

        if (c.target_path == "translation") ch.path = AnimChannel::T;
        if (c.target_path == "rotation")    ch.path = AnimChannel::R;
        if (c.target_path == "scale")       ch.path = AnimChannel::S;

        anim.channels.push_back(ch);
    }

    if (!anim.samplers.empty() &&
        !anim.samplers[0].times.empty())
        anim.duration = anim.samplers[0].times.back();

    return anim;
}

void EvaluateIdle(
    const Animation& anim,
    float time,
    std::vector<Node>& nodes)
{
    if (anim.samplers.empty())
        return;

    float t = fmod(time, anim.duration);

    for (const auto& ch : anim.channels)
    {
        if (ch.node < 0 || ch.node >= nodes.size())
            continue;

        const auto& sp = anim.samplers[ch.sampler];
        if (sp.times.size() < 2)
            continue;

        int i = 0;
        while (i + 1 < sp.times.size() && sp.times[i + 1] < t)
            i++;

        float a =
            (t - sp.times[i]) /
            (sp.times[i + 1] - sp.times[i]);

        Node& n = nodes[ch.node];

        if (ch.path == AnimChannel::T)
            n.translation = glm::mix(
                glm::vec3(sp.values[i]),
                glm::vec3(sp.values[i + 1]), a);

        else if (ch.path == AnimChannel::R)
            n.rotation = glm::slerp(
                glm::quat(sp.values[i].w, sp.values[i].x,
                    sp.values[i].y, sp.values[i].z),
                glm::quat(sp.values[i + 1].w, sp.values[i + 1].x,
                    sp.values[i + 1].y, sp.values[i + 1].z), a);
    }
}

void UpdateLocal(Node& n)
{
    n.localMatrix =
        glm::translate(glm::mat4(1), n.translation) *
        glm::mat4_cast(n.rotation) *
        glm::scale(glm::mat4(1), n.scale);
}

void UpdateGlobal(int idx, std::vector<Node>& nodes)
{
    Node& n = nodes[idx];

    if (n.parent >= 0)
        n.globalMatrix = nodes[n.parent].globalMatrix * n.localMatrix;
    else
        n.globalMatrix = n.localMatrix;

    for (int c : n.children)
        UpdateGlobal(c, nodes);
}

void BuildJointPalette(
    const Skin& skin,
    const std::vector<Node>& nodes,
    std::vector<glm::mat4>& out)
{
    out.resize(skin.joints.size());

    for (size_t i = 0; i < skin.joints.size(); i++)
        out[i] = nodes[skin.joints[i]].globalMatrix *
        skin.inverseBind[i];
}

/* =========================
   Shader Utilities
   ========================= */

static std::string LoadTextFile(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << path << "\n";
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static GLuint CompileShader(const char* path, GLenum type)
{
    std::string src = LoadTextFile(path);
    if (src.empty())
        return 0;

    GLuint shader = glCreateShader(type);
    const char* cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader compile error (" << path << "):\n"
            << log << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint LoadShaderProgram()
{
    GLuint vs = CompileShader("vertex.glsl", GL_VERTEX_SHADER);
    GLuint fs = CompileShader("fragment.glsl", GL_FRAGMENT_SHADER);

    if (!vs || !fs)
    {
        std::cerr << "Shader compilation failed\n";
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "Program link error:\n"
            << log << "\n";
        glDeleteProgram(program);
        return 0;
    }

    return program;
}
