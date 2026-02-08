#version 460 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;

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

void main()
{ 
    // post processing goes here :)))
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_ScreenTexture, TexCoords).rgb;
    vec3 exposed = hdrColor * 0.5f;
    
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