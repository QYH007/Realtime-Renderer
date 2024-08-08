#version 460 core

// --------------------------- constant variables-----------------------------------
#define EPS 1e-3

#define PI 3.141592653589793
#define PI2 6.283185307179586
#define NUM_RINGS 10

#define NUM_SAMPLES 25 //PCSS sample parameter in step 3
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES //PCSS sample parameter in step 1

const int NumLights = 2;
const float gamma     = 2.2;
const float pureWhite = 1.0;
const float ambientFactor = 0.1;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

vec4 posLightSpace;

// -------------------------- in variables -------------------------
in vec3 objectColor;
in vec3 worldPos;
in vec2 texCoord;
in mat3 tbn;
// -------------------------- textures -------------------------
uniform sampler2D shapetex;
uniform sampler2D shapetexSpec;
uniform sampler2D shapetexNormal;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;


uniform sampler2D u_DepthMap[NumLights];     //R: shadow map, G: squared shadow map
uniform sampler2D u_DepthSAT[NumLights];     //SAT map

// -------------------------- uniforms -------------------------
uniform float u_TextureSize;
uniform int shadowtype;
uniform int PCF_samples;
uniform vec3 camPos;
uniform mat4 lightSpaceMatrixes[NumLights];

// -------------------------- struct -------------------------
uniform struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
    float lightSize;

} light;

struct AnalyticalLight {
    vec3 position;
    vec3 radiance;
    float lightSize;
};

uniform AnalyticalLight lights[NumLights];

// -------------------------- OUT -------------------------
out vec3 color;
//layout(location=0) out vec4 color;


/*******-------------------- Basic calculation --------------------******/

float compute_shadow(sampler2D DepthMap, vec3 normal, vec3 lightDir)
{
	// TODO: complete the shadow evaluation

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // compare the depth value to the shadow map
    float shadow = 0.0;

	if (projCoords.z > 1.0)
    {
        shadow = 1.0;
    }
    else
    {
        float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

        if (texture(DepthMap, projCoords.xy).r > projCoords.z - bias)
        {
            shadow = 0.0;
        }
        else
        {
            shadow = 1.0;
        }
        //shadow = texture(DepthMap, projCoords.xy).r;
    }
    return shadow;
}


/*******-------------------- PCF calculation --------------------******/

float compute_shadow_PCF(sampler2D DepthMap, vec3 normal, vec3 lightDir)
{
    // 计算光源视角深度，归一化坐标
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // compare the depth value to the shadow map
    float shadow = 0.0;

    // Get texel size for shadow map
    vec2 texelSize = 1.0 / textureSize(DepthMap, 0);

    // Perform PCF by sampling neighboring texels
    for (int x = -PCF_samples; x <= PCF_samples; x++) {
        for (int y = -PCF_samples; y <= PCF_samples; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            float pcfDepth = texture(DepthMap, projCoords.xy + offset).r;
			float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
            shadow += projCoords.z > pcfDepth+bias ? 1.0 : 0.0;
        }
    }

	shadow /= ((2*PCF_samples+1) * (2*PCF_samples+1));

    return shadow;
}


/*******-------------------- VSSM functions --------------------******/

//get mean of random 2D area from SAT 
vec4 getMean(sampler2D DepthSAT, float wPenumbra, vec3 projCoords) {

	vec2 stride = 1.0 / vec2(u_TextureSize);

	float xmax = projCoords.x + wPenumbra * stride.x;
	float xmin = projCoords.x - wPenumbra * stride.x;
	float ymax = projCoords.y + wPenumbra * stride.y;
	float ymin = projCoords.y - wPenumbra * stride.y;

	vec4 A = texture(DepthSAT, vec2(xmin, ymin));
	vec4 B = texture(DepthSAT, vec2(xmax, ymin));
	vec4 C = texture(DepthSAT, vec2(xmin, ymax));
	vec4 D = texture(DepthSAT, vec2(xmax, ymax));

	float sPenumbra = 2.0 * wPenumbra;

	vec4 moments = (D + A - B - C) / float(sPenumbra * sPenumbra);

	return moments;
}

// Chebychev¡¯s inequality, use to estimate CDF, percentage of non-blockers
// in filter's area
float chebyshev(vec2 moments, float currentDepth) {
	if (currentDepth <= moments.x) {
		return 1.0;
	}
	// calculate variance from mean.
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, 0.0001);
	float d = currentDepth - moments.x;
	float p_max = variance / (variance + d * d);
	return p_max;
}


/*******-------------------- VSSM calculation --------------------******/

float compute_softshadow_VSSM(sampler2D DepthMap, sampler2D DepthSAT, vec3 normal, vec3 lightDir)
{
	float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.005);
    //float bias = 0;

	vec3 projCoords = posLightSpace.xyz / posLightSpace.w;

	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;

	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(DepthMap, projCoords.xy).r;

	float blockerSearchSize = light.lightSize/2.0f;
	float currentDepth = projCoords.z - bias;

	// keep the shadow at 1.0 when outside the zFar region of the light's frustum.
	if (currentDepth > 1.0) {
		return 1.0f;
	}

	float border = blockerSearchSize / u_TextureSize;

	// just cut out the no padding area according to the sarched area size
	if (projCoords.x <= border || projCoords.x >= 0.99f - border){
		return 1.0;
	}
	if (projCoords.y <= border || projCoords.y >= 0.99f - border) {
		return 1.0;
	}

	// Estimate average blocker depth
	vec4 moments = getMean(DepthSAT, float(blockerSearchSize), projCoords);
	//moments.x: store mean of random 2D area of shadow map
	//moments.y: store mean of random 2D area of squared shadow map

	float averageDepth = moments.x;
	float alpha = chebyshev(moments.xy, currentDepth);
	float dBlocker = (averageDepth - alpha * (currentDepth-bias)) / (1.0 - alpha);
	if (dBlocker < EPS) {
		return 0.0;
	}
	if (dBlocker > 1.0) {
		return 1.0;
	}
	float wPenumbra = (currentDepth - dBlocker) * light.lightSize / dBlocker;
	if (wPenumbra <= 0.0) {
		return 1.0;
	}
	moments = getMean(DepthSAT, wPenumbra, projCoords);
	if (currentDepth <= moments.x) {
		return 1.0;
	}

	// CDF estimation
	float shadow = chebyshev(moments.xy, currentDepth);
    
	return shadow;
}



void main()
{
	//vec3 colorMap = objectColor;
	vec3 colorMap = texture(shapetex, texCoord.xy).rgb;
	float specularMap = texture(shapetexSpec, texCoord.xy).r;

	vec3 normal = texture(shapetexNormal, texCoord.xy).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(tbn * normal);

	vec3 viewDir = vec3(normalize(camPos - worldPos));

	float ambientFactor = 0.2f;

    vec3 ambientColor = vec3(0.0);
    vec3 diffuseColor = vec3(0.0);
    vec3 specularColor = vec3(0.0);

    vec3 Lh; // 光源的半程向量
    vec3 lightDir; // 光源方向
    vec3 F; 
    float shadow = 1.0;
    vec3 Lradiance;

    for (int i=0; i<NumLights; ++i)
    {
		posLightSpace = lightSpaceMatrixes[i] * vec4(worldPos, 1);
		lightDir = normalize(lights[i].position - worldPos);

		// 环境光
		ambientColor += (lights[i].radiance.xyz * ambientFactor);

		// 漫反射
		float diffDot = max(dot(normal, lightDir), 0.0);
		vec3 diffuseTerm = diffDot * lights[i].radiance;

		// 镜面反射
		vec3 reflectDir = reflect(-lightDir, normal);
		float specDot = max(dot(viewDir, reflectDir), 0.0);
		float spec = pow(specDot, 32);
		float specStrength = 1.0;
		vec3 specularTerm = specStrength * spec * lights[i].radiance;

		// shadow mapping
        if(shadowtype == 1 ){
            shadow = 1-compute_shadow(u_DepthMap[i], normal, lightDir);
        }else if(shadowtype == 2){
            shadow = 1-compute_shadow_PCF(u_DepthMap[i], normal, lightDir);
        }else if(shadowtype == 3){
            shadow = compute_softshadow_VSSM(u_DepthMap[i], u_DepthSAT[i], normal, lightDir);
        }else{
            shadow = 1.0;
        }
	
		diffuseColor += (shadow) * diffuseTerm;
        specularColor += (shadow) * specularTerm * specularMap;


	}

	// transform to [0,1] range

	color =  (ambientColor + diffuseColor) * colorMap + specularColor;
	//color = vec4(color, 1.0);
    //color = vec3(shadow, shadow, shadow);
}

