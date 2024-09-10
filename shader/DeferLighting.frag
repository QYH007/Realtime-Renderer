#version 330 core

// --------------------------- constant variables-----------------------------------
#define EPS 1e-3

#define PI 3.141592653589793
#define PI2 6.283185307179586
#define NUM_RINGS 10

const float gamma     = 2.2;
const float pureWhite = 1.0;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);


out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gRough;
uniform sampler2D ssao;

uniform samplerCube irradianceMap;

uniform int useDiffuseTexture;

// uniform float ambientFactor;
float ambientFactor = 0.6;
uniform int showDiffuseTerm;
uniform int showColor;
uniform int showSpecular;

uniform int useAlbedoMap;
uniform int useMetalnessMap;
uniform int useRoughnessMap;
uniform int useNormalMap;

uniform int isIBL;


struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform mat4 viewMatrix;

uniform float manyLightIntensity;
uniform int outputMood;


uniform vec3 viewPos;


/*******-------------------- PBR functions --------------------******/
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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


void main()
{
    // retrieve data from gbuffer
    if(outputMood == 0){
        vec3 FragPos = texture(gPosition, TexCoords).rgb;
        vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
        vec3 normal = texture(gNormal, TexCoords.xy).rgb;

        float metalness = texture(gAlbedoSpec, TexCoords).a;

        float roughness = texture(gRough, TexCoords).r;
        float AmbientOcclusion = texture(ssao, TexCoords).r;
    
        vec3 F0 = mix(Fdielectric, albedo, metalness);

        vec3 viewDirection  = normalize( - FragPos);
                
        // then calculate lighting as usual
        vec3 directLighting = vec3(0,0,0);

        if(isIBL == 1){

            // sampling,using world normal
            mat3 viewMatrix3x3 = mat3(viewMatrix);           // 提取视图矩阵的上3x3部分
            mat3 invViewMatrix3x3 = transpose(viewMatrix3x3); // 计算视图矩阵的逆转置矩阵
            vec3 worldNormal = normalize(invViewMatrix3x3 * normal); // 转换为世界空间法线
            vec3 irradiance = texture(irradianceMap, worldNormal).rgb;
            // ambient lighting (we now use IBL as the ambient term)
            vec3 kS = fresnelSchlick(F0, max(dot(normal, viewDirection), 0.0));
            vec3 kD = 1.0 - kS;
            kD *= 1.0 - metalness;
            
            vec3 diffuse = irradiance * albedo;
            directLighting = (kD * diffuse);
        }else{
            // 简单的环境光照模拟，就是自身颜色的一个反应，跟粗糙度有关，正常来说会更复杂。
            directLighting = albedo * ambientFactor * (1.0 - roughness);
        }

        vec3 Lh; // 光源的半程向量
        vec3 lightDir; // 光源方向
        vec3 F;
        vec3 Lradiance;

        

        for(int i = 0; i < NR_LIGHTS; ++i)
        {
            // calculate distance between light source and current fragment
            vec3 light_pos = vec3(viewMatrix * vec4(lights[i].Position, 1.0));
            float distance = length(light_pos - FragPos);
            if(distance < 2*lights[i].Radius)
            {
                // diffuse
                lightDir = normalize(light_pos - FragPos);
                Lh = normalize(lightDir + viewDirection);

                // Calculate Fresnel term for direct lighting.
                F  = fresnelSchlick(F0, max(0.0, dot(Lh, viewDirection)));

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

                // attenuation
                float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
                diffuseTerm *= attenuation;
                specularTerm *= attenuation;


                directLighting += (diffuseTerm + specularTerm) * lights[i].Color * manyLightIntensity * NdotL;
                
            }
        }

        // Tone Correction
        float luminance = dot(directLighting, vec3(0.2126, 0.7152, 0.0722));
        float mappedLuminance = (luminance * (1.0 + luminance/(pureWhite*pureWhite))) / (1.0 + luminance);

        // Scale color by ratio of average luminances.
        vec3 mappedColor = (mappedLuminance / luminance) * directLighting;

        FragColor = vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);

        //FragColor = vec4(directLighting, 1.0);

        //FragColor = vec4(metalness, metalness, metalness, 1.0);
    } 
    
    if(outputMood == 1){
        vec3 FragPos = texture(gPosition, TexCoords).rgb;
        FragColor = vec4(FragPos, 1.0);
    } 
    
    if(outputMood == 2) {
        vec3 Fragalbedo = texture(gAlbedoSpec, TexCoords).rgb;
        FragColor = vec4(Fragalbedo, 1.0);
    } 
   
    if(outputMood == 3){
        vec3 normal = texture(gNormal, TexCoords).rgb;
        FragColor = vec4(normal, 1.0);
    } 
   
    if(outputMood == 4){
        float metalness = texture(gAlbedoSpec, TexCoords).a;
        FragColor = vec4(metalness, metalness, metalness, 1.0);
    } 
    
    if(outputMood == 5){
        float roughness = texture(gRough, TexCoords).r;
        FragColor = vec4(roughness,roughness,roughness, 1.0);
    } 
    
    if(outputMood == 6){
        float depth = texture(gPosition, TexCoords).a/100.0;
        FragColor = vec4(depth,depth,depth, 1.0);
    }

    if (outputMood == 7){
        float ssao = texture(ssao, TexCoords).r;
        FragColor = vec4(ssao,ssao,ssao, 1.0);
    }
    
}