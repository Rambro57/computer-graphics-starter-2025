#version 330 core

in vec3  fragmentNormal;
in vec2  fragmentUV;

uniform sampler2D uSideTex;    // GL_TEXTURE0
uniform sampler2D uTopTex;     // GL_TEXTURE1
uniform sampler2D uBottomTex;  // GL_TEXTURE2

// your lighting uniforms
uniform vec3 VIEWPOS;
struct PointLight { /* … */ };
uniform int  NUMBEROFPOINTLIGHTS;
uniform PointLight POINTLIGHTS[16];
struct DirectionalLight { /* … */ };
uniform DirectionalLight DIRECTIONALLIGHT;

out vec4 FragColor;

// (copy in your CalculateDirectionalLight / CalculatePointLight from hello_shader.fs,
//  but remove any texture lookups from MATERIAL.specular, since we’re doing them ourselves.)

void main() {
    vec3 N = normalize(fragmentNormal);
    float isTop    = step(0.9,  N.y);    // 1 on top face
    float isBottom = step(0.9, -N.y);    // 1 on bottom face
    float isSide   = 1.0 - isTop - isBottom;

    vec4 sideCol   = texture(uSideTex,   fragmentUV);
    vec4 topCol    = texture(uTopTex,    fragmentUV);
    vec4 bottomCol = texture(uBottomTex, fragmentUV);

    // pick base color
    vec4 baseColor = sideCol * isSide
                   + topCol  * isTop
                   + bottomCol * isBottom;

    if(baseColor.a < 0.1) discard;

    // now do your lighting exactly as in hello_shader.fs,
    // but use baseColor.rgb in place of texture(MATERIAL.diffuse,…).rgb
    vec3 result = CalculateDirectionalLight(DIRECTIONALLIGHT);
    for(int i = 0; i < NUMBEROFPOINTLIGHTS; i++)
        result += CalculatePointLight(POINTLIGHTS[i]);

    FragColor = vec4(baseColor.rgb * result, baseColor.a);
}
