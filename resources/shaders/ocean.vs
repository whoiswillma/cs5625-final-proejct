/**
 * Vertex shader for the geometry pass and shadow pass.
 */
#version 330

uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform sampler2D displacementMap;
uniform float displacementA, displacementB;

uniform sampler2D gradXMap;
uniform float gradXA, gradXB;

uniform sampler2D gradZMap;
uniform float gradZA, gradZB;

out vec3 wNormal;
out vec3 vPosition; // vertex position in eye space
out vec3 vNormal;   // vertex normal in eye space

void main()
{
    vec3 displaced = position;
    displaced.y += displacementA * texture(displacementMap, texCoords).r + displacementB;

    vec4 normal = -vec4(
        gradXA * texture(gradXMap, texCoords).r + gradXB,
        -1,
        gradZA * texture(gradZMap, texCoords).r + gradZB,
        0
    );

    normal = transpose(inverse(mM)) * normal;
    wNormal = normal.xyz;

    vPosition = (mV * mM * vec4(displaced, 1.0)).xyz;
    vNormal = (transpose(inverse(mV)) * normal).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
}
