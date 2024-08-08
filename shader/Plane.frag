
#version 410 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{ 
    float depthValue = texture(screenTexture, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 0.0);

    //FragColor = vec4(texture(screenTexture, TexCoords).r, texture(screenTexture, TexCoords).g, texture(screenTexture, TexCoords).b, 1.0);
}