#version 330 core

// Input vertex data
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoords;
layout(location = 4) in vec3 vTangent;

out vec3 viewPos;
out mat3 tbn;
out vec2 TexCoords;
out vec3 viewObjectNormal;

out vec4 preScreenPosition;
out vec4 nowScreenPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 mvpMatrix;

uniform mat4 preModelMatrix;
uniform mat4 preViewMatrix;
uniform mat4 preProjectionMatrix;

uniform int screenWidth;
uniform int screenHeight;
uniform int offsetIdx;

vec2 Halton_2_3[8] = vec2[8](
    vec2(0.0f, -1.0f / 3.0f),
    vec2(-1.0f / 2.0f, 1.0f / 3.0f),
    vec2(1.0f / 2.0f, -7.0f / 9.0f),
    vec2(-3.0f / 4.0f, -1.0f / 9.0f),
    vec2(1.0f / 4.0f, 5.0f / 9.0f),
    vec2(-1.0f / 4.0f, -5.0f / 9.0f),
    vec2(3.0f / 4.0f, 1.0f / 9.0f),
    vec2(-7.0f / 8.0f, 7.0f / 9.0f)
);

void main()
{
    viewPos = vec3(viewMatrix * modelMatrix * vec4(vPosition, 1.0));

    preScreenPosition = preProjectionMatrix * preViewMatrix * preModelMatrix * vec4(vPosition, 1.0);
    // 注意这里就不要添加jitter了
    nowScreenPosition = projectionMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0);


    TexCoords = vTexCoords;
    
    float deltaWidth = 1.0/screenWidth, deltaHeight = 1.0/screenHeight;
    
    viewObjectNormal = normalize(mat3(transpose(inverse(viewMatrix*modelMatrix))) * vNormal);

    vec3 b = normalize(vec3(viewMatrix * modelMatrix * vec4(cross(vNormal, vTangent), 0)));
	vec3 t = normalize(vec3(viewMatrix * modelMatrix * vec4(vTangent, 0)));
	vec3 n = normalize(vec3(viewMatrix * modelMatrix * vec4(vNormal, 0)));
	tbn = mat3(t, b, n);

    vec2 jitter = vec2(
        Halton_2_3[offsetIdx].x * deltaWidth,
        Halton_2_3[offsetIdx].y * deltaHeight
    );

    mat4 jitterMat = projectionMatrix;
    jitterMat[2][0] += jitter.x;
    jitterMat[2][1] += jitter.y;

    gl_Position = jitterMat * viewMatrix * modelMatrix * vec4(vPosition, 1);
}

