#include "MulUtil.hpp"

glm::vec3 MulUtil::mulh(glm::mat4 mat, glm::vec3 vec, float h) {
    glm::vec4 r4 = mat * glm::vec4(vec, h);
    if (r4.w != 0) {
        r4 /= r4.w;
    }
    return {r4.x, r4.y, r4.z};
}
