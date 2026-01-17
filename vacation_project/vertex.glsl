#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aJoints;  // 뼈대 인덱스 (최대 4개 영향)
layout(location = 4) in vec4 aWeights;  // 가중치

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// 최대 뼈대 개수 (모델에 따라 늘려야 할 수 있음)
const int MAX_JOINTS = 100;
uniform mat4 u_jointMatrices[MAX_JOINTS]; 

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    // 스키닝 행렬 계산 (가중치 * 해당 뼈대의 변환 행렬)
    mat4 skinMat = 
        aWeights.x * u_jointMatrices[aJoints.x] +
        aWeights.y * u_jointMatrices[aJoints.y] +
        aWeights.z * u_jointMatrices[aJoints.z] +
        aWeights.w * u_jointMatrices[aJoints.w];

    // 스키닝 행렬을 적용한 로컬 위치
    vec4 localPosition = skinMat * vec4(aPos, 1.0);
    
    // 법선 벡터도 스키닝 행렬의 회전 성분에 영향을 받아야 함
    vec4 localNormal = skinMat * vec4(aNormal, 0.0);

    gl_Position = projection * view * model * localPosition;
    
    FragPos = vec3(model * localPosition);
    Normal = mat3(transpose(inverse(model))) * localNormal.xyz;
    TexCoord = aTexCoord;
}