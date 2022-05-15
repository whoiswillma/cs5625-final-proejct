#version 330

// Uniforms
uniform sampler2D toonTex;
uniform bool shadeOcean = false;
uniform vec3 ambient;
uniform bool multipleLights;
uniform int lightNum;

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
    if (!inputs.foreground || (!shadeOcean && inputs.ocean)) {
        fragColor = vec4(0, 0, 0, 0);
    } else {
        fragColor = vec4(ambient, 1.0);

        if (texture(toonTex, geom_texCoord).a == 1) {
            float intensity = clamp(texture(toonTex, geom_texCoord).r, 0, 1);
            float specularTest = clamp(texture(toonTex, geom_texCoord).g, 0, 1);                        
            float edgeTest = clamp(texture(toonTex, geom_texCoord).b, 0, 1);

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

            if (multipleLights) {
                specularTest = clamp(texture(toonTex, geom_texCoord).g / lightNum, 0, 1);
            }
            if (specularTest > specularThreshold) {
                fragColor.xyz += pow(specularTest, pow(2, specularSmoothness * 10 + 1)) * specularIntensity;
            }

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
