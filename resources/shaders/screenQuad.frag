#version 460 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;


void main()
{ 
    // post processing goes here :)))
    float gamma = 2.2;

    vec3 originalTex = texture(screenTexture, TexCoords).rgb;
    vec3 gammaCorrected = pow(originalTex.rgb, vec3(1.0/gamma));
    FragColor = vec4(gammaCorrected, 1.0f);
}