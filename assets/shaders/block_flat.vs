#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 TRANSFORM;
uniform mat4 VIEW;
uniform mat4 PROJECTION;
uniform float TIME;
uniform bool WIND;
uniform float WINDEFFECT;

out vec3 fragmentPos;
out vec3 fragmentNormal;
out vec2 fragmentUV;

void main() {
    // Apply wind effect if enabled
    float offset = 0.0;
    if (WIND)
        offset = sin(TIME) * (aPosition.y + 0.5) * WINDEFFECT;

    vec3 windPos = aPosition + vec3(offset, 0.0, offset);
    
    // Transform position to world space
    fragmentPos = vec3(TRANSFORM * vec4(windPos, 1.0));
    
    // Transform normal into world-space (correctly handle rotation)
    fragmentNormal = mat3(transpose(inverse(TRANSFORM))) * aNormal;
    
    // Flip texture coordinates vertically to match hello_shader
    fragmentUV = vec2(aTexCoords.x, -aTexCoords.y);
    
    // Calculate final position
    gl_Position = PROJECTION * VIEW * TRANSFORM * vec4(windPos, 1.0);
}