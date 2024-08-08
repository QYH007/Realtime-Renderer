#version 460 core

// Input vertex data
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoords;
layout(location = 4) in vec3 vTangent;

out vec3 objectColor;
out vec3 worldPos;
out vec2 texCoord;
out vec3 objectNormal;
out vec3 worldObjectNormal;
out mat3 tbn;
out vec4 posLightSpace;


uniform mat4 modelMatrix;
uniform mat4 mvpMatrix;


void main()
{
	gl_Position = mvpMatrix * vec4(vPosition, 1);

	objectColor = vColor; // 应该可以不要
	texCoord = vTexCoords;
	objectNormal = vNormal;
	worldObjectNormal = normalize(mat3(transpose(inverse(modelMatrix))) * vNormal);
	worldPos = vec3(modelMatrix * vec4(vPosition, 1));

	vec3 b = normalize(vec3(modelMatrix * vec4(cross(vNormal, vTangent), 0)));
	vec3 t = normalize(vec3(modelMatrix * vec4(vTangent, 0)));
	vec3 n = normalize(vec3(modelMatrix * vec4(vNormal, 0)));
	tbn = mat3(t, b, n);
	
	// TODO: compute the posLightSpace using the lightSpaceMatrix
}