#version 460 core


out vec4 FragColor;
in vec2 TexCoords;


uniform sampler2D u_GPosition;
uniform sampler2D u_GNormal;
uniform sampler2D u_GDiffuseSpec;

void main()
{
    // Deferred Rendering now
    vec3 FragPos = texture(u_GPosition, TexCoords).rgb;
    vec3 Normal = texture(u_GNormal, TexCoords).rgb;
    vec3 Albedo = texture(u_GDiffuseSpec, TexCoords).rgb;
    float Specular = texture(u_GDiffuseSpec, TexCoords).a;

    if (length(Normal) < 0.1) 
    {
        FragColor = vec4(0.643, 0.827, 0.984, 1.0); // sky color
        //FragColor = vec4(3.0, 0.0, 0.0, 1.0);
        return;
    }

    // directional light
    vec3 lightDirection = vec3(-0.5, -0.5, -0.5);
    vec3 lightColor = vec3(2.0, 2.0, 2.0);

    float diff = max(dot(Normal, -lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0) * lightColor * Albedo;
    vec3 finalLight = diffuse + vec3(0.3, 0.3, 0.3) * Albedo + (FragPos * 0.0001f);

    vec3 newNormal = Albedo + (finalLight * 0.0001f);

    FragColor = vec4(finalLight, 1.0);

}