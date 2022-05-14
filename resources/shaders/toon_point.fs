#version 330

// Uniforms
uniform float shadowBias = 1e-3;
uniform mat4 mV;
uniform mat4 mP;
uniform vec3 wCamPos;

// Light Properties
uniform vec3 wLightPos;
uniform mat4 mV_light;
uniform mat4 mP_light;
uniform sampler2D shadowTex;
uniform vec3 ambient;
uniform bool ambientCustomized;
uniform float ambientIntensity;

// Specular Properties
uniform float specularThreshold;
uniform float specularIntensity;
uniform float specularSmoothness;

// Edge Properties
uniform float edgeThreshold;
uniform float edgeIntensity;

// Ramp Properties
uniform sampler2D ramp;
uniform bool rampEnabled;

// Stroke Properties
uniform sampler2D hatch1;
uniform sampler2D hatch2;
uniform sampler2D hatch3;
uniform sampler2D hatch4;
uniform sampler2D hatch5;
uniform sampler2D hatch6;
uniform float hatchScale = 20;
uniform float hatchThreshold = 0.5;
uniform float hatchSmoothness = 0.05;
uniform bool strokeEnabled;

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

vec3 lerp(vec3 oriColor, vec3 newColor, float alpha) {
	return (1 - alpha) * oriColor + alpha * newColor;
}

void main() {
    ShaderInput inputs = getShaderInputs(geom_texCoord);
    if (!inputs.foreground) {
        fragColor = vec4(0, 0, 0, 0);
    } else {
        vec4 fragNDC4 = vec4(inputs.fragPosNDC, 1);

        if (ambientCustomized) {
            fragColor = vec4(vec3(ambientIntensity), 1.0);
        } else {
            fragColor = vec4(ambient, 1.0);
        }

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
            if (rampEnabled) {
                fragColor.xyz += texture(ramp, vec2(intensity, 0.5)).r * inputs.diffuseReflectance;
            } else {
                if (intensity < 0.5) {
                    fragColor.xyz += 0.25 * inputs.diffuseReflectance;
                } else if (intensity < 0.75){
                    fragColor.xyz += 0.5 * inputs.diffuseReflectance;
                } else {
                    fragColor.xyz += inputs.diffuseReflectance;
                }
            }

            vec3 wCamDir = wCamPos - wPosition4.xyz;
            vec3 wHalfDir = normalize(wLightDir + wCamDir);
            float specularTest = dot(wHalfDir, inputs.normal) * 0.5 + 0.5;
            if (specularTest > specularThreshold) {
                fragColor.xyz += pow(specularTest, pow(2, specularSmoothness * 10 + 1)) * specularIntensity;
            }

            float edgeTest = dot(normalize(wCamDir), inputs.normal) * 0.5 + 0.5;
            edgeTest = 1 - edgeTest;
            if (edgeTest > edgeThreshold) {
                fragColor.xyz = edgeTest * vec3(edgeIntensity) * fragColor.xyz + (1 - edgeTest) * fragColor.xyz;
            }

            if (strokeEnabled) {
                if (intensity < hatchThreshold / 5) {
                    fragColor.xyz *= texture(hatch5, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz;
                } else if (intensity < hatchThreshold / 5 * 2) {
                    float alpha = clamp((intensity - hatchThreshold / 5) / hatchSmoothness, 0, 1);
				    fragColor.xyz = lerp(fragColor.xyz * texture(hatch5, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, 
                        fragColor.xyz * texture(hatch4, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, alpha);
                } else if (intensity < hatchThreshold / 5 * 3) {
                    float alpha = clamp((intensity - hatchThreshold / 5 * 2) / hatchSmoothness, 0, 1);
				    fragColor.xyz = lerp(fragColor.xyz * texture(hatch4, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, 
                        fragColor.xyz * texture(hatch3, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, alpha);
                } else if (intensity < hatchThreshold / 5 * 4) {
                    float alpha = clamp((intensity - hatchThreshold / 5 * 3) / hatchSmoothness, 0, 1);
				    fragColor.xyz = lerp(fragColor.xyz * texture(hatch3, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, 
                        fragColor.xyz * texture(hatch2, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, alpha);
                } else if (intensity < hatchThreshold) {
                    float alpha = clamp((intensity - hatchThreshold / 5 * 4) / hatchSmoothness, 0, 1);
				    fragColor.xyz = lerp(fragColor.xyz * texture(hatch2, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, 
                        fragColor.xyz * texture(hatch1, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, alpha);
                } else {
                    float alpha = clamp((intensity - hatchThreshold) / hatchSmoothness, 0, 1);
				    fragColor.xyz = lerp(fragColor.xyz * texture(hatch1, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz, 
                        fragColor.xyz, alpha);
                }
            }           
        } else {
            if (strokeEnabled) {
                fragColor.xyz *= texture(hatch6, geom_texCoord * hatchScale - floor(geom_texCoord * hatchScale)).xyz;
            }           
        }
    }
}
