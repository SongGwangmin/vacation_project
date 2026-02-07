#pragma once

extern GLuint cubeShaderProgram;

std::string ReadShaderFile(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void InitShaders() {
    // 1. 파일에서 소스 읽기
    std::string vsCode = ReadShaderFile("vertex.glsl");
    std::string fsCode = ReadShaderFile("fragment.glsl");

    const char* vsSource = vsCode.c_str();
    const char* fsSource = fsCode.c_str();

    GLint success;
    char infoLog[512];

    // 2. Vertex Shader 컴파일
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, NULL);
    glCompileShader(vs);

    // 컴파일 에러 체크
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 3. Fragment Shader 컴파일
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, NULL);
    glCompileShader(fs);

    // 컴파일 에러 체크
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 4. Shader Program 링크
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // 링크 에러 체크
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(shaderProgram);

    glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
}

void InitCubeShader() {
    std::string vsCode = ReadShaderFile("vertex_cube.glsl");
    std::string fsCode = ReadShaderFile("fragment_cube.glsl");

    const char* vsSource = vsCode.c_str();
    const char* fsSource = fsCode.c_str();

    GLint success;
    char infoLog[512];

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cerr << "Cube Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        std::cerr << "Cube Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    cubeShaderProgram = glCreateProgram();
    glAttachShader(cubeShaderProgram, vs);
    glAttachShader(cubeShaderProgram, fs);
    glLinkProgram(cubeShaderProgram);
    glGetProgramiv(cubeShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(cubeShaderProgram, 512, NULL, infoLog);
        std::cerr << "Cube Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}