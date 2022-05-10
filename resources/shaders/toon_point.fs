#version 330

// Uniforms
uniform float shadowBias = 1e-3;
uniform mat4 mV;
uniform mat4 mP;

// Light Properties
uniform vec3 vLightPos;
uniform mat4 mV_light;
uniform mat4 mP_light;
uniform sampler2D shadowTex;

// Specular Properties
uniform vec3 vCamPos;
uniform float specularThreshold;
uniform float specularIntensity;
uniform float specularSmoothness;

// Edge Properties
uniform float edgeThreshold;
uniform float edgeIntensity;

// Inputs
in vec2 geom_texCoord;

// Outputs
out vec4 fragColor;

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

    float shadowDepth = texture(shadowTex, shadowTexCoords).x;
    if (shadowDepth + shadowBias < shadowFragDepth) {
        fragColor = vec4(0.5 * inputs.diffuseReflectance, 1.0);
    } else {
        vec3 vLightDir = wPosition4.xyz - vLightPos;

        float intensity = dot(normalize(vLightDir), inputs.normal) * 0.5 + 0.5;
        if (intensity < 0.75) {
            fragColor = vec4(0.75 * inputs.diffuseReflectance, 1.0);
        } else {
            fragColor = vec4(inputs.diffuseReflectance, 1.0);
        }

        vec3 vCamDir = wPosition4.xyz - vCamPos;
        vec3 vHalfDir = normalize(vLightDir + vCamDir);
        float specularTest = dot(vHalfDir, inputs.normal);
        if (specularTest > specularThreshold) {
            float specular = pow(specularTest, pow(2, specularSmoothness * 10 + 1)) * specularIntensity;
            fragColor.xyz = fragColor.xyz + specular;
        }

        float edgeTest = dot(normalize(vCamDir), inputs.normal) * 0.5 + 0.5;
        if (edgeTest > edgeThreshold) {
            fragColor.xyz = edgeTest * vec3(edgeIntensity) * fragColor.xyz + (1 - edgeTest) * fragColor.xyz;
        }
    }   
}
