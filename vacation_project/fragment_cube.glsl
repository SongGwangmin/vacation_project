#version 330 core
in vec3 vColor;
in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

void main() {
    // 광원 위치
    vec3 lightPos = vec3(100.0, 100.0, 50.0);
    vec3 Filllight = vec3(50.0, 50.0, -100.0);
    // 디퓨즈 계산

    vec3 lightDir = normalize(lightPos - vWorldPos);
    vec3 normal = normalize(vNormal);
    float diff = max(dot(normal, lightDir), 0.0);

    float Filldiff = max(dot(normal, normalize(Filllight - vWorldPos)), 0.0);

    diff = diff * 0.5 + 0.5; // half lambert
    
    // 보조광 + 디퓨즈
    float ambient = 0.2;
    vec3 result = diff  * vColor;
    result += Filldiff * ambient * vColor;
    
    FragColor = vec4(result, 1.0);
}
