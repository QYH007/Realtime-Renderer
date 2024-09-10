#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gRough;
layout (location = 4) out vec4 gVelo;

in vec2 TexCoords;
in vec3 viewPos;
in vec3 viewObjectNormal;
in mat3 tbn;

in vec4 preScreenPosition;
in vec4 nowScreenPosition;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;

uniform int useAlbedoMap;
uniform int useMetalnessMap;
uniform int useRoughnessMap;
uniform int useNormalMap;
uniform int useDiffuseTexture;

uniform float near;
uniform float far;


float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // 回到NDC
    return (2.0 * near * far) / (far + near - z * (far - near));    
}


void main()
{    
    // store the fragment position vector in the first gbuffer texture
    float metalness = 0.1;
    float roughness = 0.5;

    gPosition.rgb = viewPos;

    gPosition.a = LinearizeDepth(gl_FragCoord.z); 

    // also store the per-fragment normals into the gbuffer
    vec3 normal = viewObjectNormal;

    if(useNormalMap == 1){
        normal = texture(normalTexture, TexCoords.xy).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(tbn * normal);
    }

    gNormal = normal;

    if(useDiffuseTexture == 1){
        if(useMetalnessMap == 1)
            metalness = texture(metalnessTexture, TexCoords).r;
        if(useRoughnessMap == 1)
            roughness = texture(roughnessTexture, TexCoords).r;
    }

    // and the diffuse per-fragment color
    if(useAlbedoMap == 1)
        gAlbedoSpec.rgb = texture(diffuseTexture, TexCoords).rgb;
    else
        gAlbedoSpec.rgb = vec3(0.8,0.8,0.8);

    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = metalness;

    // store roughness
    gRough.r = roughness;

    // velocity
    vec2 newPos = ((nowScreenPosition.xy / nowScreenPosition.w) * 0.5 + 0.5);
    vec2 prePos = ((preScreenPosition.xy / preScreenPosition.w) * 0.5 + 0.5);

    gVelo = vec4(newPos - prePos, 0.0, 0.0);
}