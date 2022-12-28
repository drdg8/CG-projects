#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 screenTexCoord;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float kc;
    float kl;
    float kq;
};

uniform PointLight light;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssaoResult;

void main() {
    vec3 FragPos = texture(gPosition, screenTexCoord).xyz;
    vec3 Normal = texture(gNormal, screenTexCoord).xyz;
    vec3 Diffuse = texture(gAlbedo, screenTexCoord).rgb;
    float AmbientOcclusion = texture(ssaoResult, screenTexCoord).x;

    //TODO: perform lambert shading
    // Then calculate lighting as usual
    vec3 ambient = vec3(0.3 * AmbientOcclusion); // <-- this is where we use ambient occlusion
    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // Viewpos is (0.0.0)
    // Diffuse
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.color;
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.color * spec;
    // Attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.kc + light.kl * distance + light.kq * distance * distance);
    attenuation *= light.intensity;
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    fragColor = vec4(lighting, 1.0);
}