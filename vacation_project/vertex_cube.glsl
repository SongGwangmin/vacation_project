#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;
layout (location = 2) in vec3 aScale;
layout (location = 3) in vec3 aColor;

uniform mat4 projection;
uniform mat4 view;

out vec3 vColor;

void main() {
    vec3 worldPos = aPos * aScale + aOffset;
    gl_Position = projection * view * vec4(worldPos, 1.0);
    vColor = aColor;
}
