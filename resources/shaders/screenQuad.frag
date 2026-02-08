#version 460 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;


void main()
{ 
    // post processing goes here :)))
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_ScreenTexture, TexCoords).rgb;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}