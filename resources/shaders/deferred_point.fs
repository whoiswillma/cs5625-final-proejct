/**
 * Implement shading using the brdf model in microfacet.fs
 */
#version 330

// Uniforms
uniform float shadowBias = 1e-3;
uniform mat4 mV;
uniform mat4 mP;
uniform vec2 shadowMapRes;
uniform bool pcfEnabled;

// Light Properties
uniform vec3 vLightPos; // light position in view space
uniform vec3 lightPower;
uniform mat4 mV_light;
uniform mat4 mP_light;
uniform sampler2D shadowTex;

// Inputs
in vec2 geom_texCoord;

// Outputs
out vec4 fragColor;

// HEADERS: microfacet.fs
const float PI = 3.14159265358979323846264;
float isotropicMicrofacet(vec3 i, vec3 o, vec3 n, float eta, float alpha);

// HEADERS: deferred_shader_inputs.fs
struct ShaderInput {
    bool foreground;
    vec3 diffuseReflectance;
    vec3 normal;
    float eta;
    float alpha;
    vec3 fragPosNDC;
};
ShaderInput getShaderInputs(vec2 texCoord);

float shadowTest(vec2 shadowTexCoords, float shadowFragDepth) {
   float shadowDepth = texture(shadowTex, shadowTexCoords).x;
   return shadowDepth + shadowBias < shadowFragDepth ? 0 : 1;
}

float percentCloserFiltering(vec2 shadowTexCoords, float shadowFragDepth) {
    float dx = 1 / (4 * shadowMapRes.x);
    float dy = 1 / (4 * shadowMapRes.y);
    return (
        shadowTest(shadowTexCoords + vec2(dx, dy), shadowFragDepth)
        + shadowTest(shadowTexCoords + vec2(dx, -dy), shadowFragDepth)
        + shadowTest(shadowTexCoords + vec2(-dx, dy), shadowFragDepth)
        + shadowTest(shadowTexCoords + vec2(-dx, -dy), shadowFragDepth)
    ) / 4.0;
}

void main() {
    ShaderInput inputs = getShaderInputs(geom_texCoord);
    if (!inputs.foreground) {
        fragColor = vec4(0, 0, 0, 0);
        return;
    }

    vec4 fragNDC4 = vec4(inputs.fragPosNDC, 1);

    // Position of the fragment in eye space
    vec4 vPosition4 = inverse(mP) * fragNDC4;
    vPosition4 /= vPosition4.w;
    vec3 vPosition = vPosition4.xyz;

    // Position of the fragment in world space
    vec4 wPosition4 = inverse(mV) * vPosition4;
    wPosition4 /= wPosition4.w;

    // Position of the fragment in light space
    vec4 lPosition4 = mP_light * mV_light * wPosition4;
    lPosition4 /= lPosition4.w;

    vec2 shadowTexCoords = lPosition4.xy / 2 + 0.5;
    float shadowFragDepth = lPosition4.z / 2 + 0.5;

    float shadowFactor;
    if (pcfEnabled) {
        shadowFactor = percentCloserFiltering(shadowTexCoords, shadowFragDepth);
    } else {
        float shadowDepth = texture(shadowTex, shadowTexCoords).x;
        shadowFactor = shadowDepth + shadowBias < shadowFragDepth ? 0 : 1;
    }

    vec3 positionToLight = vLightPos - vPosition;
    vec3 incomingDirection = normalize(positionToLight);
    vec3 positionToEye = vec3(0, 0, 0) - vPosition;
    vec3 outgoingDirection = normalize(positionToEye);

    float specular = isotropicMicrofacet(
        incomingDirection,
        outgoingDirection,
        inputs.normal,
        inputs.eta,
        inputs.alpha
    );

    vec3 brdf = inputs.diffuseReflectance / PI + specular * vec3(1, 1, 1);

    vec3 fragColor3 = shadowFactor *
        lightPower / (4 * PI)
        * brdf
        * max(0.0, dot(inputs.normal, incomingDirection))
        / dot(positionToLight, positionToLight);
    fragColor = vec4(fragColor3, 1.0);
}
