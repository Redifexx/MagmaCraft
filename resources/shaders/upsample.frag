#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_Input;
uniform float u_FilterRadius;


// using advanced warfare's bloom technique
// Taken from Lava Engine WebGL

void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = u_FilterRadius;
    float y = u_FilterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    
    vec3 a = texture(u_Input, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 b = texture(u_Input, vec2(TexCoords.x,     TexCoords.y + y)).rgb;
    vec3 c = texture(u_Input, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;

    vec3 d = texture(u_Input, vec2(TexCoords.x - x, TexCoords.y)).rgb;
    vec3 e = texture(u_Input, vec2(TexCoords.x,     TexCoords.y)).rgb;
    vec3 f = texture(u_Input, vec2(TexCoords.x + x, TexCoords.y)).rgb;

    vec3 g = texture(u_Input, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 h = texture(u_Input, vec2(TexCoords.x,     TexCoords.y - y)).rgb;
    vec3 i = texture(u_Input, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |

    vec3 upsample3 = e * 4.0;
    upsample3 += (b+d+f+h)*2.0;
    upsample3 += (a+c+g+i);
    upsample3 *= 1.0 / 16.0;

    FragColor = vec4(upsample3, 1.0);
}