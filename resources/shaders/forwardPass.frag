#version 460 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_ASMETexture;
uniform sampler2D u_ShadowMap;
uniform mat4 u_LightSpaceMatrix;
uniform vec3 u_CameraPosition;
uniform vec3 u_SunColor;
uniform vec3 u_SunDirection;
uniform float u_SunIntensity;
uniform float u_ShadowBiasMin;
uniform float u_ShadowBiasMax;
uniform float u_ShadowFadeDistance;
uniform float u_AmbientIntensity;

const float PI = 3.14159265359;

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

// learn opengl's pbr
// normal distribution function that approximates the relative suface area of
// microfacets exactly aligned to the halfway vector
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

// geometry function that approximates the relative surface area where its micro
// surface details overshadow each other, causing light rays to be occluded
// using a combination of both schlick-ggx & smith's functions
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

//
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// fresnel schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


void main()
{
    // Deferred Lighting
    vec3 Normal = texture(u_NormalTexture, TexCoords).rgb;
    Normal = Normal * 2.0 - 1.0;
    Normal = normalize(TBN * Normal);
    vec3 norm = normalize(Normal);

    vec3 Albedo = texture(u_AlbedoTexture, TexCoords).rgb;
    float Transparency = texture(u_AlbedoTexture, TexCoords).a;

    if (Transparency < 0.01f)
    {
        discard;
    }

    float AmbientOcclusion = texture(u_ASMETexture, TexCoords).r;
    float Smoothness = texture(u_ASMETexture, TexCoords).g;
    float Metallic = texture(u_ASMETexture, TexCoords).b;
    float Emissive = texture(u_ASMETexture, TexCoords).a;

    if (length(Normal) < 0.1) 
    {
        discard;
    }

    vec3 viewDir = normalize(u_CameraPosition - FragPos);

    // Shadow map stuff
    vec4 FragPosLightSpace = u_LightSpaceMatrix * vec4(FragPos, 1.0f);

    // directional light
    vec3 lightDirection = -u_SunDirection; // to light
    vec3 lightColor = u_SunColor;
    float lightIntensity = u_SunIntensity;

    vec3 F0 = vec3(0.04f); // base reflectivity
    F0 = mix(F0, Albedo, Metallic);

    vec3 Lo = vec3(0.0f);

    // do for each light eventually
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    vec3 radiance = lightColor * lightIntensity; // no attentuation here
    float roughness = 1.0 - Smoothness;

    // cook-torrance brdf
    float NDF = DistributionGGX(norm, halfwayDir, roughness);
    float G = GeometrySmith(norm, viewDir, lightDirection, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(norm, viewDir), 0.0) * max(dot(norm, lightDirection), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // energy conservation
    vec3 kSpec = F;
    vec3 kDiff = vec3(1.0) - kSpec;
    kDiff *= 1.0 - Metallic;

    float NdotL = max(dot(norm, lightDirection), 0.0);

    // add to outgoing radiance
    Lo += (kDiff * Albedo / PI + specular) * radiance * NdotL;

    // shadows
    float shadow = ShadowCalculation(FragPosLightSpace, norm, -lightDirection);
    float dist = length(FragPos - u_CameraPosition);
    float fade = smoothstep(u_ShadowFadeDistance * 0.8, u_ShadowFadeDistance, dist);
    shadow *= (1.0 - fade);

    vec3 ambient = vec3(u_AmbientIntensity) * Albedo * AmbientOcclusion; // until IBL

    vec3 emissiveLight = Albedo * Emissive * 3.0;
    vec3 color = (Lo * (1.0 - shadow)) + ambient + emissiveLight;

    FragColor = vec4(color, Transparency);

}