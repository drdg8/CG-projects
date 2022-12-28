#version 330 core
layout(location = 0) out float ssaoResult;

const int nSamples = 64;
const float radius = 1.0f;

uniform int screenWidth;
uniform int screenHeight;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noiseMap;
uniform vec3 sampleVecs[64];
uniform mat4 projection;

in vec2 screenTexCoord;

const vec2 noiseScale = vec2(screenWidth/4.0f, screenHeight/4.0f); 

void main() {
    // TODO: perform SSAO
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 randomVec = texture(noiseMap, TexCoords * noiseScale).xyz;
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < nSamples; ++i)
    {
        // get sample position
        vec3 sample = TBN * sampleVecs[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = -texture(gPosition, offset.xy).w; // Get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / nSamples);
    
    ssaoResult = occlusion;
}