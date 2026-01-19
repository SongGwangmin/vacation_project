#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in ivec4 aBoneIds;
layout (location = 3) in vec4 aWeights;

uniform mat4 finalBonesMatrices[100];
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    vec4 totalPos = vec4(0.0);
    bool hasBones = false;

    for(int i = 0; i < 4; i++) {
        if(aBoneIds[i] == -1) continue;
        hasBones = true;
        vec4 localPos = finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1.0);
        totalPos += localPos * aWeights[i];
    }

    if(!hasBones) {
        totalPos = vec4(aPos, 1.0);
    }

    gl_Position = projection * view * model * totalPos;
    TexCoord = aTex;
}