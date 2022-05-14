/**
 * Computes radiance for background pixels using the sunsky model in sunsky.fs
 */
#version 330

// HEADERS: sunsky.fs
vec3 sunskyRadiance(vec3 dir);

// Uniforms
uniform vec3 background;
uniform sampler2D image;
uniform mat4 mP;
uniform mat4 mV;

// Inputs
in vec2 geom_texCoord;

// Outputs
out vec4 fragColor;

vec3 eyeDirection(vec2 xyNDC) {
    mat4 mP_inv = inverse(mP);
    vec4 near = mP_inv * vec4(xyNDC, -1, 1);
    near /= near.w;
    vec4 far = mP_inv * vec4(xyNDC, 1, 1);
    far /= far.w;
    return normalize(far.xyz - near.xyz);
}

void main() {
    vec4 color = texture(image, geom_texCoord);
    if (color.w != 0) {
        fragColor = color;
        return;
    }

    vec2 xyNDC = 2 * geom_texCoord - 1;
    vec4 eyeDirection4 = vec4(eyeDirection(xyNDC), 0);
    vec4 worldDirection4 = inverse(mV) * eyeDirection4;
    vec3 worldDirection = normalize(worldDirection4.xyz);

    if (worldDirection.y > 0) {
        fragColor = vec4(sunskyRadiance(worldDirection), 1);
    } else {
        fragColor = vec4(background, 1);
    }
}
