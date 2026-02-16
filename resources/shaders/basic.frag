#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gASME;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_ASMETexture;


void main()
{
    // loading textures into the GBuffer
    
    gPosition = FragPos;

    vec3 normal = texture(u_NormalTexture, TexCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);
    gNormal = normal;

    gASME = texture(u_ASMETexture, TexCoords);

    gAlbedo = texture(u_AlbedoTexture, TexCoords);
    
}
