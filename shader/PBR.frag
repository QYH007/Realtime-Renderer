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

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

vec4 posLightSpace;

// -------------------------- in variables -------------------------
in vec3 objectColor;
in vec3 worldPos;
in vec2 texCoord;
in vec3 objectNormal;
in vec3 worldObjectNormal;
in mat3 tbn;


// -------------------------- textures -------------------------
uniform sampler2D shapetex;
uniform sampler2D shapetexSpec;
uniform sampler2D shapetexNormal;

// PBR
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;


uniform sampler2D u_DepthMap[NumLights];     //R: shadow map, G: squared shadow map
uniform sampler2D u_DepthSAT[NumLights];     //SAT map

// -------------------------- uniforms -------------------------
uniform float u_TextureSize;
uniform int shadowtype;
uniform int PCF_samples;
uniform vec3 camPos;
uniform mat4 lightSpaceMatrixes[NumLights];
uniform int useDiffuseTexture;

uniform float envIntensity;
uniform float defaultRoughness;
uniform float defaultMetalness;


uniform int showNormal;
uniform int isEnvironmentLight;
uniform int showColor;
uniform int isDirectLight;

uniform int useAlbedoMap;
uniform int useMetalnessMap;
uniform int useRoughnessMap;
uniform int useNormalMap;

uniform int isIBL;


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

layout(location=0) out vec4 Fragcolor;


/*******-------------------- PBR functions --------------------******/
// F
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta,  float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

// D
float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    a2 = a2*a2;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

// G
float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
        
    return nom / denom;
}

// GG
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
        
    return ggx1 * ggx2;
}

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
        float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.005);
       // float bias = 0.00;

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

float compute_shadow_PCF(sampler2D u_DepthMap, vec3 normal, vec3 lightDir)
{
    // 计算光源视角深度，归一化坐标
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // compare the depth value to the shadow map
    float shadow = 0.0;

    // Get texel size for shadow map
    vec2 texelSize = 1.0 / textureSize(u_DepthMap, 0);
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.01);
	//bias = 0;

    // Perform PCF by sampling neighboring texels
    for (int x = -PCF_samples; x <= PCF_samples; x++) {
        for (int y = -PCF_samples; y <= PCF_samples; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            float pcfDepth = texture(u_DepthMap, projCoords.xy + offset).r;
			
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
	float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.001);
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
    // -------------------read texture-----------------
	//vec3 albedo = objectColor;
    vec3 albedo = vec3(0.8, 0.5, 0.5);
    float metalness = defaultMetalness;
    float roughness = defaultRoughness;
    vec3 normal = worldObjectNormal;
    vec3 F0 = vec3(1.0);

    
    if(useDiffuseTexture == 1){
        if(useAlbedoMap == 1 && showColor == 1)
            albedo = texture(diffuseTexture, texCoord).rgb;
        if(useMetalnessMap == 1)
		    metalness = texture(metalnessTexture, texCoord).r;
            
        if(useRoughnessMap == 1)
            roughness = texture(roughnessTexture, texCoord).r;
            
        if(useNormalMap == 1){
            normal = texture(normalTexture, texCoord.xy).rgb;
            normal = normalize(normal * 2.0 - 1.0);
            normal = normalize(tbn * normal);
        }
    }

    F0 = mix(Fdielectric, albedo, metalness);
    

    // ------------- vectors --------------
    vec3 viewDirection = normalize(camPos - worldPos);
    vec3 Lh; // 光源的半程向量
    vec3 lightDir; // 光源方向

    vec3 reflectDir = reflect(-viewDirection, normal); // 视线的反射方向

    // shading begin
    // 金属度插值，要么非金属，要么纯金属
    vec3 ambient = vec3(0,0,0);
    vec3 Lo = vec3(0,0,0);
    vec3 specular = vec3(0,0,0);

    // ambient term
 
    // ambient lighting (we now use IBL as the ambient term)
    vec3 kS = fresnelSchlickRoughness(F0, max(dot(normal, viewDirection), 0.0), roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;

    vec3 irradiance = texture(irradianceMap, normal).rgb;
    vec3 diffuse = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    //
    vec3 prefilteredColor = textureLod(prefilterMap, reflectDir, roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(normal, viewDirection), 0.0), roughness)).rg;
    specular = prefilteredColor * (kS * brdf.x + brdf.y);

    ambient = (kD * diffuse + specular) * envIntensity;
    
    
    vec3 F; 
    float shadow = 1.0;
    vec3 Lradiance;
	// 对每一个光源做累加计算
    for (int i=0; i<NumLights; ++i)
    {
        posLightSpace = lightSpaceMatrixes[i] * vec4(worldPos, 1);
        lightDir = normalize(lights[i].position - worldPos);
        Lradiance = lights[i].radiance;

        // Half-vector between Li and viewDirection.
        Lh = normalize(lightDir + viewDirection);

        // Calculate Fresnel term for direct lighting.
        F  = fresnelSchlick(F0, max(0.0, dot(Lh, viewDirection)));

        /* TODO: Implement the normal distribution function (GGX NDF), the joint masking shadowing function and combine
        them with the provided Fresnel to the BRDF for the specular term. Also add the BRDF for the diffuse term. pay
        attention to energy conservation. Eventually add everyhting to the directLighting
        */

        // BRDF
        float D = DistributionGGX(normal, Lh, roughness);

        float k = (roughness +1.0) * (roughness+1.0) / 8.0; 

        float G = GeometrySmith(normal, viewDirection, lightDir, k);

        float NdotV = max(dot(normal, viewDirection), 0.0);
        float NdotL = max(dot(normal, lightDir), 0.0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness;
    
        vec3 specularTerm = D * G * F / (4.0 * NdotV * NdotL + EPS);
        vec3 diffuseTerm = kD * albedo / PI ;

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


        Lo += (diffuseTerm + specularTerm) * Lradiance * NdotL * shadow;
        // END TODO
    }


    vec3 color = vec3(0,0,0);
    if(isEnvironmentLight == 1){
        if(isIBL == 1)
            color += ambient;
        else
            color +=albedo * envIntensity * (1.0 - roughness);
    }
    
    if(isDirectLight == 1)
        color += Lo;


    // Tone Correction
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance/(pureWhite*pureWhite))) / (1.0 + luminance);

    // Scale color by ratio of average luminances.
    vec3 mappedColor = (mappedLuminance / luminance) * color;
	Fragcolor = vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);

    if(showNormal == 1){
        Fragcolor = vec4(normal, 1.0);
    }

    // Fragcolor = vec4(specular, 1.0);
    // Fragcolor = vec4(0.2,0.3,0.5,1.0);
    // Fragcolor = vec4(albedo, 1.0);
	// Fragcolor = vec4(lightDir,1.0);
	// Fragcolor = vec4(shadow, 1.0, 1.0, 1.0);

}

