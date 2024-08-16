#version 330 core

// Input vertex data
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoords;
layout(location = 4) in vec3 vTangent;

out vec3 veiwPos;
out vec2 TexCoords;
out vec3 veiwObjectNormal;
out mat3 tbn;


uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 mvpMatrix;

void main()
{
    veiwPos = vec3(viewMatrix * modelMatrix * vec4(vPosition, 1.0));

    TexCoords = vTexCoords;
    
    veiwObjectNormal = normalize(mat3(transpose(inverse(viewMatrix*modelMatrix))) * vNormal);

    vec3 b = normalize(vec3(viewMatrix * modelMatrix * vec4(cross(vNormal, vTangent), 0)));
	vec3 t = normalize(vec3(viewMatrix * modelMatrix * vec4(vTangent, 0)));
	vec3 n = normalize(vec3(viewMatrix * modelMatrix * vec4(vNormal, 0)));
	tbn = mat3(t, b, n);
        
    gl_Position = mvpMatrix * vec4(vPosition, 1);
}