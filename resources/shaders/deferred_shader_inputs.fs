/**
 * Library shader that includes functions to extract material properties from the geometry pass.
 *   - This shader is the complement of deferred_geom.fs
 */
#version 330

struct ShaderInput {
    bool foreground;
    vec3 diffuseReflectance;
    vec3 normal; // Fragment normal in eye-space
    float eta;
    float alpha;
    vec3 fragPosNDC; // Fragment coordinates in NDC
};

uniform vec2 viewportSize;
uniform sampler2D depthTex;
uniform sampler2D diffuseReflectanceTex;
uniform sampler2D materialTex;
uniform sampler2D normalsTex;

vec3 getFragPosNDC(vec2 texCoord) {
    float depthSS = texture(depthTex, texCoord).x;

    // [0, viewportSize.x] x [0, viewportSize.y] x [0, 1]
    vec3 fragPos = vec3(gl_FragCoord.x, gl_FragCoord.y, depthSS);

    // [0, 1] x [0, 1] x [0, 1]
    fragPos.x /= viewportSize.x;
    fragPos.y /= viewportSize.y;

    // [-1, 1] x [-1, 1] x [-1, 1]
    fragPos = 2 * fragPos - 1;

    return fragPos;
}

ShaderInput getShaderInputs(vec2 texCoord) {
    ShaderInput result;

    result.foreground = texture(normalsTex, texCoord).w != 0;
    result.diffuseReflectance = texture(diffuseReflectanceTex, texCoord).xyz;
    result.normal = normalize(2 * texture(normalsTex, texCoord).xyz - 1);

    vec4 material = texture(materialTex, texCoord);
    result.eta = 10 * material.x;
    result.alpha = material.y;

    result.fragPosNDC = getFragPosNDC(texCoord);

    return result;
}
