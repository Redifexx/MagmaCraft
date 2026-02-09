#version 460 core


out vec4 FragColor;
in vec2 TexCoords;


uniform sampler2D u_GPosition;
uniform sampler2D u_GNormal;
uniform sampler2D u_GAlbedo;
uniform sampler2D u_GASME;
uniform vec3 u_CameraPosition;
uniform vec3 u_SkyColor;
uniform vec3 u_SunColor;
uniform vec3 u_SunDirection;
uniform float u_SunIntensity;

/*
CURRENT TEXTURE MAPS
- Albedo RGB, Transparency A (RGBA)
- Normal +Y (RGB)
- AO (R), Smoothness (G), Metallic (B), Emission (A)
*/

void main()
{
    // Deferred Lighting
    vec3 FragPos = texture(u_GPosition, TexCoords).rgb;
    vec3 Normal = texture(u_GNormal, TexCoords).rgb;
    vec3 Albedo = texture(u_GAlbedo, TexCoords).rgb;
    float Transparency = texture(u_GAlbedo, TexCoords).a;

    // prepping for pbr later
    float AmbientOcclusion = texture(u_GASME, TexCoords).r;
    float Smoothness = texture(u_GASME, TexCoords).g;
    float Metallic = texture(u_GASME, TexCoords).b;
    float Emissive = texture(u_GASME, TexCoords).a;

    if (length(Normal) < 0.1) 
    {
        FragColor = vec4(u_SkyColor, 1.0);
        return;
    }

    vec3 N = normalize(Normal);

    // directional light
    vec3 lightDirection = u_SunDirection;
    vec3 lightColor = u_SunColor;
    float lightIntensity = u_SunIntensity;

    float diff = max(dot(N, -lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0) * lightColor * Albedo;

    vec3 viewDir = normalize(u_CameraPosition - FragPos);

    vec3 halfwayDir = normalize(-lightDirection + viewDir);

    float shininess = mix(2.0, 128.0, Smoothness);
    float spec = pow(max(dot(N, halfwayDir), 0.0), shininess);
    vec3 specularLight = lightColor * spec * Smoothness;

    vec3 emissiveLight = Albedo * Emissive * 3.0;


    vec3 finalLight = ((lightIntensity * (diffuse + specularLight)) + (vec3(0.3) * Albedo) + emissiveLight);

    FragColor = vec4(finalLight, 1.0);

}