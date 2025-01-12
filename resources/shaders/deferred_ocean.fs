/**
 * ACADEMIC INTEGRITY: based off Section 4.3 of Simulating Ocean Water by Jerry Tessendorf
 */
#version 330

// HEADERS: deferred_shader_inputs.fs
struct ShaderInput {
    bool foreground;
    bool ocean;
    vec3 diffuseReflectance;
    vec3 normal;
    float eta;
    float alpha;
    vec3 fragPosNDC;
};
ShaderInput getShaderInputs(vec2 texCoord);

// Uniforms
uniform vec3 upwelling;
uniform float renderDistance = 150;
const float nSnell = 1.34;

uniform mat4 mP;
uniform mat4 mV;

// Inputs
in vec2 geom_texCoord;

// Outputs
out vec4 fragColor;

// HEADERS: sunsky.fs
vec3 sunskyRadiance(vec3 dir);

void main() {
    ShaderInput inputs = getShaderInputs(geom_texCoord);
    if (!inputs.foreground || !inputs.ocean) {
        fragColor = vec4(0, 0, 0, 0);
        return;
    }

    vec4 fragNDC4 = vec4(inputs.fragPosNDC, 1);

    // Position of the fragment in eye space
    vec4 vPosition4 = inverse(mP) * fragNDC4;
    vPosition4 /= vPosition4.w;
    vec3 vPosition = vPosition4.xyz;

    vec3 nN = normalize(inputs.normal);
    vec3 nI = normalize(vPosition);

    float costhetai = abs(dot(nI, nN));
    float thetai = acos(costhetai);
    float sinthetat = sin(thetai) / nSnell;
    float thetat = asin(sinthetat);

    float reflectivity;
    if (thetai == 0) {
        reflectivity = (nSnell - 1) / (nSnell + 1);
        reflectivity = reflectivity * reflectivity;
    } else {
        float fs = sin(thetat - thetai) / sin(thetat + thetai);
        float ts = tan(thetat - thetai) / tan(thetat + thetai);
        reflectivity = 0.5 * (fs * fs + ts * ts);
    }

    // Have the reflectivity fall off as a function of (distance / renderDistance)
    reflectivity = min(exp(-length(vPosition) / renderDistance), reflectivity);

    vec4 vReflectedDirection = vec4(normalize(-nI - 2 * nN * dot(-nI, nN)), 0);
    vec3 sky = sunskyRadiance((inverse(mV) * vReflectedDirection).xyz);

    vec3 fragColor3 = (reflectivity * sky + (1 - reflectivity) * upwelling);
    fragColor = vec4(fragColor3, 1);
}
