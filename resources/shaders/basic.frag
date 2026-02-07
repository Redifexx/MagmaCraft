#version 460 core

in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D u_Texture;
layout (location = 0) out vec4 FragColor;

void main()
{
    // Simple Directional Light Setup, diffuse only
    vec3 lightDirection = vec3(-0.5, -0.5, -0.5);
    vec3 lightColor = vec3(2.0, 2.0, 2.0);

    vec3 diffuseTexture = texture(u_Texture, TexCoords).rgb;

    float diff = max(dot(Normal, -lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0) * lightColor * diffuseTexture;

    vec3 finalLight = diffuse + vec3(0.3, 0.3, 0.3) * diffuseTexture;

    FragColor = vec4(finalLight, 1.0);
}