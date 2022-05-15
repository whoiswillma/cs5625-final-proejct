#version 330

// Uniforms
uniform float shadowBias = 1e-3;
uniform mat4 mV;
uniform mat4 mP;
uniform vec3 wCamPos;
uniform bool shadeOcean = false;

// Light Properties
uniform vec3 wLightPos;
uniform mat4 mV_light;
uniform mat4 mP_light;
uniform sampler2D shadowTex;

// Inputs
in vec2 geom_texCoord;

// Outputs
out vec4 fragColor;

// HEADERS: deferred_shader_inputs.fs
struct ShaderInput {
    bool foreground;
    bool ocean;
    vec3 diffuseReflectance;
    vec3 normal; // Fragment normal in eye-space
    float eta;
    float alpha;
    vec3 fragPosNDC; // Fragment coordinates in NDC
};
ShaderInput getShaderInputs(vec2 texCoord);

void main() {
    ShaderInput inputs = getShaderInputs(geom_texCoord);
    if (!inputs.foreground || (!shadeOcean && inputs.ocean)) {
        fragColor = vec4(0, 0, 0, 0);
    } else {
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

        float shadowDepth = texture(shadowTex, shadowTexCoords).x;
        if (shadowDepth + shadowBias >= shadowFragDepth) {
            vec3 wLightDir = wLightPos - wPosition4.xyz;
            float intensity = dot(normalize(wLightDir), inputs.normal) * 0.5 + 0.5;

            vec3 wCamDir = wCamPos - wPosition4.xyz;
            vec3 wHalfDir = normalize(wLightDir + wCamDir);
            float specularTest = dot(wHalfDir, inputs.normal) * 0.5 + 0.5;

            float edgeTest = dot(normalize(wCamDir), inputs.normal) * 0.5 + 0.5;
            edgeTest = 1 - edgeTest;

            fragColor = vec4(intensity, specularTest, edgeTest, 1);
        } else {
            fragColor = vec4(0);
        }
    }
}
