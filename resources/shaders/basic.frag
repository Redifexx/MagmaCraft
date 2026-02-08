#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuseSpec;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D u_DiffuseSpecTexture;
uniform sampler2D u_NormalTexture;

void main()
{
    // Deferred Rendering now
    
    gPosition = FragPos;

    vec3 normal = texture(u_NormalTexture, TexCoords).rgb;
    //normal.g = 1.0 - normal.g;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);
    gNormal = normal;

    gDiffuseSpec = texture(u_DiffuseSpecTexture, TexCoords);
    
}
