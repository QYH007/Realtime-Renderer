#version 460 core

// Input vertex data
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoords;
layout(location = 4) in vec3 vTangent;

out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 mvpMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform int outlineWidth;
uniform vec3 camPos;

uniform int useNormalMap;

// PBR
uniform sampler2D normalTexture;



void main()
{
    vec3 worldNormal = normalize(mat3(transpose(inverse(modelMatrix))) * vNormal);
    texCoord = vTexCoords;

    vec4 ndcPos = mvpMatrix * vec4(vPosition, 1);

    vec3 b = normalize(vec3(modelMatrix * vec4(cross(vNormal, vTangent), 0)));
	vec3 t = normalize(vec3(modelMatrix * vec4(vTangent, 0)));
	vec3 n = normalize(vec3(modelMatrix * vec4(vNormal, 0)));
	mat3 tbn = mat3(t, b, n);

    if(useNormalMap == 1){
        worldNormal = texture(normalTexture, vTexCoords.xy).rgb;
        worldNormal = normalize(worldNormal * 2.0 - 1.0);
        worldNormal = normalize(tbn * worldNormal);
    }

    // 将法向量转换到裁剪空间（使用视图-投影矩阵）
    mat4 vpMatrix = projectionMatrix * viewMatrix;
    vec4 clipNormal = vpMatrix * vec4(worldNormal, 0.0);  
    clipNormal = normalize(clipNormal);  // 归一化

    vec4 clipPos = mvpMatrix * vec4(vPosition, 1.0);
    
    float dist = length(camPos - vec3(modelMatrix * vec4(vPosition, 1.0)));

    clipPos.xy += clipNormal.xy * outlineWidth * clamp(dist * 0.2, 0.0, 1.0) * 0.01;

	gl_Position = clipPos;

	// TODO: compute the posLightSpace using the lightSpaceMatrix
}