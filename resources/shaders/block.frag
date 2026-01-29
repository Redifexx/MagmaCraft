#version 460 core

in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D u_Texture;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec3 diffuseTexture = texture(u_Texture, TexCoords).rgb;
    //vec3 debugColor = normalize(Normal) * 0.5 + 0.5 + (diffuseTexture * 0.0001f);
    FragColor = vec4(diffuseTexture, 1.0);
}