#version 330 core
in vec3 vColor;
in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

void main() {
    // 광원 위치
    vec3 lightPos = vec3(1.0, 1.0, 1.0);
    
    // 디퓨즈 계산
    vec3 lightDir = normalize(lightPos - vWorldPos);
    vec3 normal = normalize(vNormal);
    float diff = max(dot(normal, lightDir), 0.3);
    
    // 주변광 + 디퓨즈
    //float ambient = 0.3;
    vec3 result = diff * vColor;
    
    FragColor = vec4(result, 1.0);
}
