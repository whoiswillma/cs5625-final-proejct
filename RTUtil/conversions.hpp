/*
 * Cornell CS5625
 * RTUtil library
 * 
 * Some handy type conversion utilities for working with Assimp in a
 * program where you want to use Eigen as the principal matrix/vector
 * library.
 *
 * Author: srm, Spring 2020
 */

#pragma once

#include <Eigen/Core>
#include <assimp/scene.h>
#include <assimp/anim.h>


namespace RTUtil {

    inline glm::vec2 a2g(const aiVector2D &v) {
        return glm::vec2(v.x, v.y);
    }

    inline glm::vec3 a2g(const aiVector3D &v) {
        return glm::vec3(v.x, v.y, v.z);
    }

    inline glm::vec4 a2g(const aiVector4D &v) {
        return glm::vec4(v.x, v.y, v.z, v.w);
    }

    inline glm::vec3 a2g(const aiColor3D &c) {
        return glm::vec3(c.r, c.g, c.b);
    }

    inline glm::vec4 a2g(const aiColor4D &c) {
        return glm::vec4(c.r, c.g, c.b, c.a);
    }

    inline glm::quat a2g(const aiQuaternion &q) {
        return glm::quat(q.w, q.x, q.y, q.z);
    }

    inline glm::mat3 a2g(const aiMatrix3x3 &m) {
        return glm::mat3(
            m.a1, m.b1, m.c1, // first column
            m.a2, m.b2, m.c2, 
            m.a3, m.b3, m.c3, // last column
            );
    }

    inline glm::mat4 a2g(const aiMatrix4x4 &m) {
        return glm::mat4(
            m.a1, m.b1, m.c1, m.d1, // first column
            m.a2, m.b2, m.c2, m.d2, 
            m.a3, m.b3, m.c3, m.d3,
            m.a4, m.b4, m.c4, m.d4, // last column
            );
    }

}