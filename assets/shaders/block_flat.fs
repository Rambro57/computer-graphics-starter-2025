#version 330 core

in vec3 fragmentPos;
in vec3 fragmentNormal;
in vec2 fragmentUV;

uniform sampler2D uSideTex;    // GL_TEXTURE0
uniform sampler2D uTopTex;     // GL_TEXTURE1
uniform sampler2D uBottomTex;  // GL_TEXTURE2
uniform vec3 COLOR;
uniform vec3 VIEWPOS;
uniform float TIME;

// Light structures
struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

// Material properties
struct Material {
    float shininess;
};

// Uniforms for lighting
uniform DirectionalLight DIRECTIONALLIGHT;
uniform PointLight POINTLIGHTS[4];
uniform int NUMBEROFPOINTLIGHTS;
uniform Material MATERIAL;

out vec4 FragColor;

// Function declarations
vec3 CalculateDirectionalLight(DirectionalLight light, vec4 baseColor);
vec3 CalculatePointLight(PointLight light, vec4 baseColor);

void main() {
    vec3 N = normalize(fragmentNormal);
    vec4 baseColor;

    // Determine which texture to use based on normal direction
    if (N.y > 0.9) {
        // Top face
        baseColor = texture(uTopTex, fragmentUV);
    } 
    else if (N.y < -0.9) {
        // Bottom face
        baseColor = texture(uBottomTex, fragmentUV);
    } 
    else {
        // Side faces
        baseColor = texture(uSideTex, fragmentUV);
    }

    // Apply color tint
    baseColor *= vec4(COLOR, 1.0);

    // Discard transparent pixels
    if (baseColor.a <= 0.0) {
        discard;
    }

    // Calculate lighting
    vec3 result = CalculateDirectionalLight(DIRECTIONALLIGHT, baseColor);

    // Add point light contributions
    for(int i = 0; i < NUMBEROFPOINTLIGHTS; i++) {
        result += CalculatePointLight(POINTLIGHTS[i], baseColor);
    }

    FragColor = vec4(result * baseColor.rgb, baseColor.a);
}

// Implementation of lighting calculation functions
vec3 CalculateDirectionalLight(DirectionalLight light, vec4 baseColor) {
    // ambient
    vec3 ambient = light.ambient;
  	
    // diffuse 
    vec3 norm = normalize(fragmentNormal);
    vec3 lightDir = normalize(-light.direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;  
    
    // specular
    vec3 viewDir = normalize(VIEWPOS - fragmentPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), MATERIAL.shininess);
    vec3 specular = light.specular * spec * 0.3; // modest specular value
        
    return ambient + diffuse + specular;
}

vec3 CalculatePointLight(PointLight light, vec4 baseColor) {
    // ambient
    vec3 ambient = light.ambient;
  	
    // diffuse 
    vec3 norm = normalize(fragmentNormal);
    vec3 lightDir = normalize(light.position - fragmentPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;  
    
    // specular
    vec3 viewDir = normalize(VIEWPOS - fragmentPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), MATERIAL.shininess);
    vec3 specular = light.specular * spec * 0.3; // modest specular value
    
    // attenuation
    float distance = length(light.position - fragmentPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                        light.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse  *= attenuation;
    specular *= attenuation;   
        
    return ambient + diffuse + specular;
}