#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aOffset;
layout (location = 3) in vec3 aScale;
layout (location = 4) in vec3 aColor;

uniform mat4 projection;
uniform mat4 view;

out vec3 vColor;
out vec3 vNormal;
out vec3 vWorldPos;

void main() {
    vec3 worldPos = aPos * aScale + aOffset;
    gl_Position = projection * view * vec4(worldPos, 1.0);
    vColor = aColor;
    // 노말은 스케일에 의해 변형될 수 있으므로 정규화 (비균등 스케일 대응)
    vNormal = normalize(aNormal / aScale);
    vWorldPos = worldPos;
}
