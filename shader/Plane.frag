
#version 410 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{ 
    vec4 texColor = texture(screenTexture, TexCoords);
    //FragColor = vec4(vec3(texColor), 0.0);
    //FragColor = vec4(0.5, 0.6, 0.6, 0.0);

    FragColor = vec4(texColor.r, texColor.r, texColor.r, 1.0);
    //FragColor = texColor;
}