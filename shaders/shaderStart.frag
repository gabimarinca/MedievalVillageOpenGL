#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
uniform bool enableFog;
out vec4 fColor;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

//textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float computeFog() 
{ 
    float fogDensity = 0.01f; 
    float fragmentDistance = length(fPosEye); 
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2)); 
    return clamp(fogFactor, 0.0f, 1.0f); 
}

float computeShadow()
{
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    if (normalizedCoords.z > 1.0f)
        return 0.0f;
    return shadow;
}

void computeLightComponents()
{        
    vec3 cameraPosEye = vec3(0.0f); // In eye coordinates, the viewer is situated at the origin
    
    // Transform normal
    vec3 normalEye = normalize(fNormal);    
    
    // Compute light direction
    vec3 lightDirN = normalize(lightDir);
    
    // Compute view direction 
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
        
    // Compute ambient light
    ambient = ambientStrength * lightColor;
    
    // Compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    
    // Compute specular light
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

void main() 
{
    computeLightComponents();
    
    // Calculate fog factor based on distance
    float fogFactor = computeFog();

    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    // Calculate the final color after applying fog
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    // Compute shadow
    float shadow = computeShadow();
    vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
    
    // Apply fog by mixing the color with the fog color based on the fog factor
    fColor = mix(fogColor, vec4(color, 1.0f), fogFactor); 
}
