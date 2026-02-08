#version 460 core


out vec4 FragColor;
in vec2 TexCoords;


uniform sampler2D u_GPosition;
uniform sampler2D u_GNormal;
uniform sampler2D u_GDiffuseSpec;
uniform vec3 u_CameraPosition;

void main()
{
    // Deferred Rendering now
    vec3 FragPos = texture(u_GPosition, TexCoords).rgb;
    vec3 Normal = texture(u_GNormal, TexCoords).rgb;
    vec3 Albedo = texture(u_GDiffuseSpec, TexCoords).rgb;
    float Specular = texture(u_GDiffuseSpec, TexCoords).a;

    if (length(Normal) < 0.1) 
    {
        FragColor = vec4(0.2, 0.5, 0.9, 1.0);
        return;
    }

    vec3 N = normalize(Normal);

    // directional light
    vec3 lightDirection = vec3(-0.5, -0.5, -0.5);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float lightIntensity = 2.0f;

    float diff = max(dot(N, -lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0) * lightColor * Albedo;

    vec3 viewDir = normalize(u_CameraPosition - FragPos);

    vec3 halfwayDir = normalize(-lightDirection + viewDir);

    float spec = pow(max(dot(N, halfwayDir), 0.0), 8.0);
    vec3 specularLight = lightColor * spec * Specular * 0.5;

    vec3 finalLight = ((lightIntensity * (diffuse + specularLight)) + (vec3(0.3, 0.3, 0.3) * Albedo));

    FragColor = vec4(finalLight, 1.0);

}