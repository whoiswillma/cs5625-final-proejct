/*
 * Cornell CS5625
 *
 * Some geometric computations useful for Monte Carlo illumination.
 * Implementations borrowed from Mitsuba 0.5.0 by Wenzel Jakob.
 * https://www.mitsuba-renderer.org
 */
#define _USE_MATH_DEFINES

#include <cmath>
#include "geomtools.hpp"

namespace RTUtil {

glm::vec2 squareToUniformDiskConcentric(const glm::vec2 &sample) {
    float r1 = 2.0f * sample.x - 1.0f;
    float r2 = 2.0f * sample.y - 1.0f;

    /* Modified concencric map code with less branching (by Dave Cline), see
       http://psgraphics.blogspot.ch/2011/01/improved-code-for-concentric-map.html */
    float phi, r;
    if (r1 == 0 && r2 == 0) {
        r = phi = 0;
    } else if (r1*r1 > r2*r2) {
        r = r1;
        phi = (M_PI/4.0f) * (r2/r1);
    } else {
        r = r2;
        phi = (M_PI/2.0f) - (r1/r2) * (M_PI/4.0f);
    }

    float cosPhi, sinPhi;
    math::sincos(phi, &sinPhi, &cosPhi);

    return glm::vec2(r * cosPhi, r * sinPhi);
}

#define EXPECT_NOT_TAKEN(a) (a)

glm::vec3 squareToCosineHemisphere(const glm::vec2 &sample) {
    glm::vec2 p = squareToUniformDiskConcentric(sample);
    float z = math::safe_sqrt(1.0f - p.x * p.x - p.y * p.y);

    /* Guard against numerical imprecisions */
    if (EXPECT_NOT_TAKEN(z == 0))
        z = 1e-10f;

    return glm::vec3(p.x, p.y, z);
}

}
