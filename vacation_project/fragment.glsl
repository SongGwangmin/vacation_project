#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

void main()
{
    // 간단한 조명 처리 (디버깅용)
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 objectColor = vec3(1.0, 1.0, 1.0); // 회색
    vec3 result = (0.3 + diff) * objectColor; // Ambient + Diffuse

    FragColor = vec4(result, 1.0);
}