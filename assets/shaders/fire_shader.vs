#version 330 core

in vec3 aPosition;
in vec3 aNormal;
in vec2 aUV;

out vec2 fragmentUV;
out vec3 fragmentPos;
out vec3 fragmentNormal;

uniform mat4 TRANSFORM;
uniform mat4 VIEW;
uniform mat4 PROJECTION;
uniform float TIME;

void main()
{
    // Add subtle movement to the vertices based on time for a dancing fire effect
    float yOffset = sin(aPosition.x * 2.0 + TIME * 2.0) * 0.1;
    float xOffset = cos(aPosition.z * 2.0 + TIME * 2.0) * 0.1;
    
    // Apply the offset more to higher parts of the flame
    float heightFactor = max(0.0, aPosition.y);
    vec3 modifiedPosition = aPosition + vec3(xOffset * heightFactor, yOffset * heightFactor, 0.0);
    
    // Transform the vertex
    fragmentPos = vec3(TRANSFORM * vec4(modifiedPosition, 1.0));
    fragmentNormal = aNormal;
    fragmentUV = vec2(aUV.x, -aUV.y);
    gl_Position = PROJECTION * VIEW * vec4(fragmentPos, 1.0);
}