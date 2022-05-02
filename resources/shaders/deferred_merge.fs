/**
 * Implements merging of images for the bloom effect.
 */
#version 330

in vec2 geom_texCoord;

out vec4 fragColor;

uniform sampler2D image;
uniform sampler2D blurred;

void main() {
    vec3 fragColor3 =
        0.8843 * texture(image, geom_texCoord).xyz
        + 0.1 * textureLod(blurred, geom_texCoord, 2).xyz
        + 0.012 * textureLod(blurred, geom_texCoord, 4).xyz
        + 0.0027 * textureLod(blurred, geom_texCoord, 6).xyz
        + 0.001 * textureLod(blurred, geom_texCoord, 8).xyz;
    fragColor = vec4(fragColor3, 1);
}

