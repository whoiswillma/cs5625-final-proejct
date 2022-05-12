/**
 * Fragment shader for the geometry pass.
 *   - Encodes the diffuse reflectance, eta, alpha, and normal of the geometry.
 *   - deferred_shader_inputs.fs extracts the encoded geometry properties.
 */
#version 330

// Material properties
uniform float alpha;
uniform float eta;
uniform vec3 diffuseReflectance;

// Inputs
in vec3 vPosition;
in vec3 vNormal;

// Outputs
layout (location = 0) out vec4 outDiffuseReflectance;
layout (location = 1) out vec4 outMaterial;
layout (location = 2) out vec4 outNormal;

void main() {
    vec3 normal = normalize((gl_FrontFacing) ? vNormal : -vNormal);

    outDiffuseReflectance = vec4(diffuseReflectance, 1);
    outMaterial = vec4(eta / 10, alpha, 0, 1);
    outNormal = vec4(normal / 2 + 0.5, 1);
}