#version 330

// Material properties
uniform float alpha;
uniform float eta;
uniform vec3 diffuseReflectance;

// Light properties
uniform vec3 lightPower;
uniform vec3 vLightPos;

// Inputs
in vec3 vPosition;
in vec3 wNormal;

// Outputs
out vec4 fragColor;

float linearToSrgb(float x) {
    if (x <= 0.0031308) {
        return 12.92 * x;
    } else {
        return 1.055 * pow(x, 1 / 2.4) - 0.055;
    }
}

vec3 linearToSrgb3(vec3 x) {
    return vec3(
        linearToSrgb(x.x),
        linearToSrgb(x.y),
        linearToSrgb(x.z)
    );
}

void main() {
    vec3 fragColor3 = (normalize(wNormal) + 1) / 2;
    fragColor = vec4(fragColor3, 1.0);
}