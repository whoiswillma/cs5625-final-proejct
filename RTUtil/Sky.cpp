#define _USE_MATH_DEFINES
#include <cmath>

#include "Sky.hpp"

using namespace RTUtil;

void Sky::init() {
    cY = mat2x5(
        vec5(0.1787f, -0.3554f, -0.0227f, 0.1206f, -0.0670f),
        vec5(-1.4630f, 0.4275f, 5.3251f, -2.5771f, 0.3703f)
    );
    cx = mat2x5(
        vec5(-0.0193f, -0.0665f, -0.0004f, -0.0641f, -0.0033f),
        vec5(-0.2592f, 0.0008f, 0.2125f, -0.8989f, 0.0452f)
    );
    cy = mat2x5(
        vec5(-0.0167f, -0.0950f, -0.0079f, -0.0441f, -0.0109f),
        vec5(-0.2608f, 0.0092f, 0.2102f, -1.6537f, 0.0529f)
    );
     
    Mx = glm::mat4x3(
         0.0017f, -0.0290f,  0.1169f,
        -0.0037f,  0.0638f, -0.2120f,
         0.0021f, -0.0320f,  0.0605f,
         0.0000f,  0.0039f,  0.2589f
    );
    My = glm::mat4x3(
         0.0028f, -0.0421f,  0.1535f,
        -0.0061f,  0.0897f, -0.2676f,
         0.0032f, -0.0415f,  0.0667f,
         0.0000f,  0.0052f,  0.2669f
    ); 
}


// Compute the zenith luminance for given sky parameters
static float Y_z(float theta, float T) {
    float chi = (4./9. - T/120) * (M_PI - 2 * theta);
    return (4.0453 * T - 4.9710) * tan(chi) - 0.2155 * T + 2.4192;
}

// Compute the zenith chromaticity x or y for given sky
// parameters, given the matrix that defines Preetham's
// polynomial approximation for that component.
static float __z(float theta, float T, const glm::mat4x3& M) {
    glm::vec3 vT = glm::vec3(T * T, T, 1.0f);
    glm::vec4 vth = glm::vec4(theta * theta * theta, theta * theta, theta, 1.0f);
    return glm::dot(vT * M, vth);
}

void Sky::setUniforms(GLWrap::Program &prog) {

    // Compute the parameters A, ..., E to the Perez model.  There is
    // a separate set of parameters for each of the three color 
    // components Y, x, and y.
    vec5 pY = cY * glm::vec2(turbidity, 1.0f);
    vec5 px = cx * glm::vec2(turbidity, 1.0f);
    vec5 py = cy * glm::vec2(turbidity, 1.0f);

    // Compute the zenith (looking straight up) color, which is
    // used to set the correct overall scale for each color component.
    float Yz = Y_z(theta_sun, turbidity);
    float xz = __z(theta_sun, turbidity, Mx);
    float yz = __z(theta_sun, turbidity, My);

    // Pass these results to the fragment shader
    prog.uniform("A", glm::vec3(pY.x, px.x, py.x));
    prog.uniform("B", glm::vec3(pY.y, px.y, py.y));
    prog.uniform("C", glm::vec3(pY.z, px.z, py.z));
    prog.uniform("D", glm::vec3(pY.w, px.w, py.w));
    prog.uniform("E", glm::vec3(pY.v, px.v, py.v));
    prog.uniform("zenith", glm::vec3(Yz, xz, yz));
    prog.uniform("thetaSun", theta_sun);
}