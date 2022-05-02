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
in vec3 vNormal;

// Outputs
out vec4 fragColor;

// HEADERS: microfacet.fs
const float PI = 3.14159265358979323846264;
float isotropicMicrofacet(vec3 i, vec3 o, vec3 n, float eta, float alpha);

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
    vec3 positionToLight = vLightPos - vPosition;
    vec3 incomingDirection = normalize(positionToLight);
    vec3 positionToEye = vec3(0, 0, 0) - vPosition;
    vec3 outgoingDirection = normalize(positionToEye);
    vec3 normal = normalize((gl_FrontFacing) ? vNormal : -vNormal);

    float specular = isotropicMicrofacet(
        incomingDirection,
        outgoingDirection,
        normal,
        eta,
        alpha
    );

    vec3 brdf = diffuseReflectance / PI + specular * vec3(1, 1, 1);

    vec3 fragColor3 =
        lightPower / (4 * PI)
        * brdf
        * max(0.0, dot(normal, incomingDirection))
        / dot(positionToLight, positionToLight);
    fragColor = vec4(linearToSrgb3(fragColor3), 1.0);
}