#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <GL/glew.h>     // GLEW (반드시 gl/freeglut.h 보다 먼저 포함)
#include <GL/freeglut.h> // FreeGLUT
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include <tinygltf-release/tiny_gltf.h>

#include <iostream>
#include <vector>
#include <string>

// --- 데이터 구조체 (이전과 동일) ---
struct Node {
    int parentIndex = -1;
    std::vector<int> children;
    glm::mat4 globalMatrix = glm::mat4(1.0f);

    // 초기 트랜스폼 데이터
    glm::vec3 translation = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

// --- 전역 변수 (FreeGLUT 콜백에서 접근하기 위해 전역 필요) ---
tinygltf::Model gltfModel;
std::vector<Node> nodes;
std::vector<glm::mat4> inverseBindMatrices;
std::vector<glm::mat4> jointMatrices;
GLuint shaderProgram;

// --- 유틸리티: 셰이더 로드 (구현 생략) ---
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {
    // 1. 셰이더 코드를 파일에서 읽어오기
    std::string VertexShaderCode;
    std::string FragmentShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);

    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        std::cerr << "ERROR: 버텍스 셰이더 파일을 열 수 없습니다: " << vertex_file_path << std::endl;
        return 0;
    }

    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }
    else {
        std::cerr << "ERROR: 프래그먼트 셰이더 파일을 열 수 없습니다: " << fragment_file_path << std::endl;
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // 2. 버텍스 셰이더 컴파일
    std::cout << "Compiling Vertex Shader: " << vertex_file_path << std::endl;
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // 버텍스 셰이더 에러 체크
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        std::cerr << &VertexShaderErrorMessage[0] << std::endl;
    }

    // 3. 프래그먼트 셰이더 컴파일
    std::cout << "Compiling Fragment Shader: " << fragment_file_path << std::endl;
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // 프래그먼트 셰이더 에러 체크
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        std::cerr << &FragmentShaderErrorMessage[0] << std::endl;
    }

    // 4. 프로그램 링크
    std::cout << "Linking Program..." << std::endl;
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // 링크 에러 체크
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << &ProgramErrorMessage[0] << std::endl;
    }

    // 5. 컴파일된 셰이더 객체 삭제 (프로그램에 링크되었으므로 더 이상 개별 객체는 필요 없음)
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

// --- 1. GLB 로드 및 파싱 (이전과 논리 동일) ---
bool LoadGLB(const std::string& filename) {
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    bool ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);

    if (!warn.empty()) std::cout << "Warn: " << warn << std::endl;
    if (!err.empty()) std::cout << "Err: " << err << std::endl;
    if (!ret) return false;

    // 노드 초기화
    nodes.resize(gltfModel.nodes.size());
    for (size_t i = 0; i < gltfModel.nodes.size(); i++) {
        const auto& gNode = gltfModel.nodes[i];
        nodes[i].children = gNode.children;
        for (int childId : gNode.children) nodes[childId].parentIndex = (int)i;

        if (gNode.translation.size() == 3) nodes[i].translation = glm::make_vec3(gNode.translation.data());
        if (gNode.rotation.size() == 4) nodes[i].rotation = glm::make_quat(gNode.rotation.data());
        if (gNode.scale.size() == 3) nodes[i].scale = glm::make_vec3(gNode.scale.data());
    }

    // 스킨 데이터 로드
    if (gltfModel.skins.size() > 0) {
        const auto& skin = gltfModel.skins[0];
        const auto& accessor = gltfModel.accessors[skin.inverseBindMatrices];
        const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
        const auto& buffer = gltfModel.buffers[bufferView.buffer];

        inverseBindMatrices.resize(skin.joints.size());
        jointMatrices.resize(skin.joints.size());

        const float* data = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        for (size_t i = 0; i < skin.joints.size(); i++) {
            inverseBindMatrices[i] = glm::make_mat4(data + i * 16);
        }
    }
    return true;
}

// --- 2. 노드 계층 업데이트 ---
void UpdateNodeHierarchy(int nodeIndex, const glm::mat4& parentTransform) {
    Node& node = nodes[nodeIndex];

    glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), node.translation) *
        glm::mat4(node.rotation) *
        glm::scale(glm::mat4(1.0f), node.scale);

    node.globalMatrix = parentTransform * localTransform;

    for (int childIndex : node.children) {
        UpdateNodeHierarchy(childIndex, node.globalMatrix);
    }
}

// --- 3. 애니메이션 업데이트 ---
void UpdateAnimation(float time) {
    if (gltfModel.animations.empty()) return;
    const auto& anim = gltfModel.animations[0];

    for (const auto& channel : anim.channels) {
        const auto& sampler = anim.samplers[channel.sampler];
        const auto& inputAcc = gltfModel.accessors[sampler.input];
        const auto& inputView = gltfModel.bufferViews[inputAcc.bufferView];
        const float* inputData = reinterpret_cast<const float*>(&gltfModel.buffers[inputView.buffer].data[inputView.byteOffset + inputAcc.byteOffset]);

        const auto& outputAcc = gltfModel.accessors[sampler.output];
        const auto& outputView = gltfModel.bufferViews[outputAcc.bufferView];
        const float* outputData = reinterpret_cast<const float*>(&gltfModel.buffers[outputView.buffer].data[outputView.byteOffset + outputAcc.byteOffset]);

        float maxTime = inputData[inputAcc.count - 1];
        float curTime = fmod(time, maxTime);

        int keyIndex = 0;
        for (int i = 0; i < inputAcc.count - 1; i++) {
            if (curTime >= inputData[i] && curTime < inputData[i + 1]) {
                keyIndex = i;
                break;
            }
        }

        Node& node = nodes[channel.target_node];
        if (channel.target_path == "translation") node.translation = glm::make_vec3(&outputData[keyIndex * 3]);
        else if (channel.target_path == "rotation") node.rotation = glm::make_quat(&outputData[keyIndex * 4]);
        else if (channel.target_path == "scale") node.scale = glm::make_vec3(&outputData[keyIndex * 3]);
    }

    int rootNodeIndex = gltfModel.scenes[gltfModel.defaultScene].nodes[0];
    UpdateNodeHierarchy(rootNodeIndex, glm::mat4(1.0f));

    if (!gltfModel.skins.empty()) {
        const auto& skin = gltfModel.skins[0];
        for (size_t i = 0; i < skin.joints.size(); i++) {
            int jointNodeIndex = skin.joints[i];
            jointMatrices[i] = nodes[jointNodeIndex].globalMatrix * inverseBindMatrices[i];
        }
    }
}

// --- FreeGLUT Display 콜백 ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // 1. 애니메이션 시간 계산 (밀리초 -> 초)
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    UpdateAnimation(time);

    // 2. 유니폼 전송
    if (!jointMatrices.empty()) {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_jointMatrices"),
            jointMatrices.size(), GL_FALSE, glm::value_ptr(jointMatrices[0]));
    }

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 2, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

    // 3. 그리기 (VAO 바인딩 및 DrawCall)
    // TODO: glDrawElements(...) 호출

    glutSwapBuffers();
}

// --- FreeGLUT Timer 콜백 (애니메이션 루프용) ---
void timer(int value) {
    glutPostRedisplay(); // 화면 갱신 요청 (display 함수 호출됨)
    glutTimerFunc(16, timer, 0); // 약 60FPS (16ms 후 재호출)
}

int main(int argc, char** argv) {
    // 1. FreeGLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("GLB Animation with GLEW & FreeGLUT");

    // 2. GLEW 초기화 (반드시 창 생성 후)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // 3. 데이터 로드 및 설정
    shaderProgram = LoadShaders("vertex.glsl", "fragment.glsl");

    if (shaderProgram == 0) {
        std::cerr << "셰이더 로드 실패!" << std::endl;
        return -1;
    }

    if (!LoadGLB("peto.glb")) return -1;

    // TODO: 여기서 VAO/VBO 생성 및 데이터 업로드 수행

    glEnable(GL_DEPTH_TEST);

    // 4. 콜백 등록
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);

    // 5. 메인 루프 진입
    glutMainLoop();

    return 0;
}