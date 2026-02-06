#version 460 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;


void main()
{ 
    // post processing goes here :)))
    FragColor = texture(screenTexture, TexCoords);
}