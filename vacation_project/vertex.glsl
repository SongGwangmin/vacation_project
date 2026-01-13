#version 330 core

/* =========================
   Attributes (glTF 표준)
   ========================= */

layout(location = 0) in vec3  aPos;       // POSITION
layout(location = 1) in vec3  aNormal;    // NORMAL
layout(location = 2) in uvec4 aJoints;    // JOINTS_0
layout(location = 3) in vec4  aWeights;   // WEIGHTS_0
layout(location = 4) in vec2  aUV;        // TEXCOORD_0

/* =========================
   Uniforms
   ========================= */

uniform mat4 uMVP;
uniform mat4 uJoints[64];   // 본 개수 최대치 (필요시 늘려라)

/* =========================
   Varyings
   ========================= */

out vec2 vUV;
out vec3 vNormal;

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

    /* ---- Outputs ---- */

    gl_Position = uMVP * skinnedPos;

    vUV = aUV;
    vNormal = mat3(skinMat) * aNormal;
}
