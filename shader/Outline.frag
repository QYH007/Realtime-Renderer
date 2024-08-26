#version 460 core

// PBR

uniform sampler2D diffuseTexture;

in vec3 objectColor;
in vec2 texCoord;

layout(location=0) out vec4 Fragcolor;



void main()
{
    vec3 albedo = texture(diffuseTexture, texCoord.xy).rgb;
    Fragcolor = vec4(albedo, 1.0);
    //Fragcolor = vec4(0.1, 0.1, 0.1, 1.0);
}
