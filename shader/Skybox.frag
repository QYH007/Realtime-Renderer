#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform float envIntensity;

void main()
{    
    FragColor = texture(skybox, TexCoords) * (envIntensity+0.05);
}