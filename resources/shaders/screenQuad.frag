#version 460 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;
uniform sampler2D u_DepthTexture;
uniform vec3 u_SkyColor;
uniform float u_FogFar;
uniform float u_FogNear;
uniform float u_FogDensity;
uniform float u_FogCurve;

vec3 ACESFilm(vec3 x)
{
    // ACES approximation by Narkowicz
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * u_FogNear * u_FogFar) / (u_FogFar + u_FogNear - z * (u_FogFar - u_FogNear));	
}

void main()
{ 
    // post processing goes here :)))
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_ScreenTexture, TexCoords).rgb;
    float linearDepth = LinearizeDepth(texture(u_DepthTexture, TexCoords).r);

    float density = u_FogDensity;
    float fogDistance = max(linearDepth - 20.0, 0.0);
    float fogFactor = exp(-pow(fogDistance * density, u_FogCurve));
    fogFactor = clamp(fogFactor, 0.0, 1.0); // 1.0 is clear and 0.0 is fog

    vec3 foggedColor = mix(u_SkyColor, hdrColor, fogFactor);


    vec3 exposed = foggedColor * 0.5f;
    
    // reinhard tone mapping
    //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    vec3 mapped = ACESFilm(exposed);
    float saturation = 1.0;
    float luma = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
    
    vec3 saturated = mix(vec3(luma), mapped, saturation);

  

    // gamma correction 
    vec3 finalColor = pow(saturated, vec3(1.0 / gamma));

    FragColor = vec4(finalColor, 1.0);
}