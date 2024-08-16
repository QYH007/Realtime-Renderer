#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 worldPos;
in vec3 worldObjectNormal;
in mat3 tbn;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;

uniform int useAlbedoMap;
uniform int useMetalnessMap;
uniform int useRoughnessMap;
uniform int useNormalMap;


void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = worldPos;

    // also store the per-fragment normals into the gbuffer
    vec3 normal = worldObjectNormal;
    if(useNormalMap == 1){
        normal = texture(normalTexture, TexCoords.xy).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(tbn * normal);
    }
    gNormal = normal;

    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(diffuseTexture, TexCoords).rgb;

    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(metalnessTexture, TexCoords).r;
}