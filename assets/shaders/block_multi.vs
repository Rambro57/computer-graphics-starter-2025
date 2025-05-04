#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

out vec3 fragmentNormal;
out vec2 fragmentUV;
out vec3 fragmentPos;

uniform mat4 TRANSFORM;
uniform mat4 VIEW;
uniform mat4 PROJECTION;

void main() {
    fragmentNormal = mat3(transpose(inverse(TRANSFORM))) * aNormal;
    fragmentUV     = aUV;
    fragmentPos    = vec3(TRANSFORM * vec4(aPosition,1.0));
    gl_Position    = PROJECTION * VIEW * TRANSFORM * vec4(aPosition,1.0);
}
