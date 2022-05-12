/**
 * Vertex shader for the geometry pass and shadow pass.
 */
#version 330

uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix


layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform bool useBones = false;
uniform mat4 boneTransforms[100];
layout (location = 2) in ivec4 boneIds;
layout (location = 3) in vec4 boneWts;

out vec3 vPosition; // vertex position in eye space
out vec3 vNormal;   // vertex normal in eye space

void main()
{
    mat4 modelMatrix = mat4(0);

    if (useBones) {
        for (int i = 0; i < 4; i++) {
            if (boneIds[i] != -1) {
                modelMatrix += boneWts[i] * boneTransforms[boneIds[i]];
            }
        }
    } else {
        modelMatrix = mM;
    }

    vec4 position4 = vec4(position, 1);
    position4 = mV * modelMatrix * position4;
    position4 /= position4.w;
    vPosition = position4.xyz;

    vNormal = (transpose(inverse(mV * modelMatrix)) * vec4(normal, 0.0)).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
}
