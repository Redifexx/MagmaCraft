#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;


layout (location = 0) uniform mat4 u_ViewProjection;
layout (location = 1) uniform mat4 u_Model;       
layout (location = 2) uniform mat3 u_NormalMatrix;       

out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;

void main()
{
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    gl_Position = u_ViewProjection * worldPos;
    
    vec3 n = normalize(u_NormalMatrix * aNormal);
    vec3 t = normalize(u_NormalMatrix * aTangent);
    vec3 b = normalize(u_NormalMatrix * aBitangent);
    t = normalize(t - dot(t, n) * n);
    b = cross(n, t);

    TBN = mat3(t, b, n);

    TexCoords = aTexCoords;
}