#version 330 core
//The fragment shader for the fire effect - simplified version
out vec4 FragColor;

struct Material {
    sampler2D diffuse;  // Single texture for current animation frame
    sampler2D specular;
    float shininess;
};

in vec2 fragmentUV;
in vec3 fragmentPos;
in vec3 fragmentNormal;

uniform vec3 COLOR;
uniform Material MATERIAL;
uniform float TIME;

void main() {
    // Get the current texture color
    vec4 texColor = texture(MATERIAL.diffuse, fragmentUV);
    
    // Discard fully transparent pixels
    if (texColor.a < 0.1)
        discard;
    
    // Make fire fully self-illuminated - no lighting calculation needed
    // This ensures fire is always visible regardless of scene lighting
    vec3 fireColor = texColor.rgb * COLOR;
    
    // Add emissive glow to fire
    vec3 glowColor = vec3(1.0, 0.7, 0.3); // Warm orange glow
    
    // Create flickering effect using sine function for subtle movement
    float flicker = 0.9 + 0.1 * sin(TIME * 10.0);
    
    // Final color with glow and flicker
    vec3 finalColor = fireColor * flicker + fireColor * glowColor * 0.5;
    
    FragColor = vec4(finalColor, texColor.a);
}