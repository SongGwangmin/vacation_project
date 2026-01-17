#version 330 core

/* =========================
   Attributes (glTF 표준)
   ========================= */

layout(location = 0) in vec3  aPos;       // POSITION
layout(location = 1) in vec3  aNormal;    // NORMAL
layout(location = 2) in uvec4 aJoints;    // JOINTS_0 (unsigned int)
layout(location = 3) in vec4  aWeights;   // WEIGHTS_0
layout(location = 4) in vec2  aTexCoord;  // TEXCOORD_0

/* =========================
   Uniforms
   ========================= */

uniform mat4 uMVP;
uniform mat4 uJoints[64];   // 본 개수 최대치

/* =========================
   Outputs
   ========================= */

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

/* =========================
   Main
   ========================= */

void main()
{
    /* ---- Skinning ---- */
    mat4 skinMat =
        aWeights.x * uJoints[aJoints.x] +
        aWeights.y * uJoints[aJoints.y] +
        aWeights.z * uJoints[aJoints.z] +
        aWeights.w * uJoints[aJoints.w];

    vec4 skinnedPos = skinMat * vec4(aPos, 1.0);
    vec3 skinnedNorm = mat3(skinMat) * aNormal;

    /* ---- Outputs ---- */
    gl_Position = uMVP * skinnedPos;

    FragPos = vec3(skinnedPos);
    Normal = skinnedNorm;
    TexCoord = aTexCoord;
}