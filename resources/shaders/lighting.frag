#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_GPosition;
uniform sampler2D u_GNormal;
uniform sampler2D u_GAlbedo;
uniform sampler2D u_GASME;
uniform sampler2D u_ShadowMap;
uniform mat4 u_LightSpaceMatrix;
uniform vec3 u_CameraPosition;
uniform vec3 u_SkyColor;
uniform vec3 u_SunColor;
uniform vec3 u_SunDirection;
uniform float u_SunIntensity;
uniform float u_ShadowBiasMin;
uniform float u_ShadowBiasMax;
uniform float u_ShadowFadeDistance;

/*
CURRENT TEXTURE MAPS
- Albedo RGB, Transparency A (RGBA)
- Normal +Y (RGB)
- AO (R), Smoothness (G), Metallic (B), Emission (A)
*/

float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm, vec3 lightDir)
{
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // shadow bias
    float bias = max(u_ShadowBiasMax * (1.0 - dot(norm, lightDir)), u_ShadowBiasMin);

    if (projCoords.z > 1.0) return 0.0;

    // percentage closer filtering
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0f;

    return shadow;
}

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

    // Shadow map stuff
    vec4 FragPosLightSpace = u_LightSpaceMatrix * vec4(FragPos, 1.0f);

    if (length(Normal) < 0.1) 
    {
        FragColor = vec4(u_SkyColor, 1.0);
        return;
    }

    vec3 norm = normalize(Normal);

    // directional light
    vec3 lightDirection = u_SunDirection;
    vec3 lightColor = u_SunColor;
    float lightIntensity = u_SunIntensity;

    float diff = max(dot(norm, -lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0) * lightColor * Albedo;

    vec3 viewDir = normalize(u_CameraPosition - FragPos);

    vec3 halfwayDir = normalize(-lightDirection + viewDir);

    float shininess = mix(2.0, 128.0, Smoothness);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specularLight = lightColor * spec * Smoothness;

    vec3 emissiveLight = Albedo * Emissive * 3.0;

    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDirection);
    float dist = length(FragPos - u_CameraPosition);
    float fade = smoothstep(u_ShadowFadeDistance * 0.8, u_ShadowFadeDistance, dist);
    shadow *= (1.0 - fade);


    vec3 finalLight = ((lightIntensity * (diffuse + specularLight)) * (1.0 - shadow) + (vec3(0.3) * Albedo) + emissiveLight);

    FragColor = vec4(finalLight, 1.0);

}