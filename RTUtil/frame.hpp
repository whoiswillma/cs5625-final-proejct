/*
    Cornell CS5625
    Microfacet model reference interface

    This file originates from Nori, a simple educational ray tracer

    Copyright (c) 2012-2020 by Wenzel Jakob and Steve Marschner.

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined(__FRAME_H)
#define __FRAME_H

#include <glm/glm.hpp>

#include "common.hpp"
#include "geomtools.hpp"

namespace nori {
/**
 * \brief Stores a three-dimensional orthonormal coordinate frame
 *
 * This class is mostly used to quickly convert between different
 * cartesian coordinate systems and to efficiently compute certain
 * quantities (e.g. \ref cosTheta(), \ref tanTheta, ..).
 */
struct RTUTIL_EXPORT Frame {
    glm::vec3 s, t;
    glm::vec3 n;

    /// Default constructor -- performs no initialization!
    inline Frame() { }

    /// Given a normal and tangent vectors, construct a new coordinate frame
    inline Frame(const glm::vec3 &s, const glm::vec3 &t, const glm::vec3 &n)
     : s(s), t(t), n(n) { }

    /// Construct a new coordinate frame from a single vector
    inline Frame(const glm::vec3 &n) : n(n) {
        coordinateSystem(n, s, t);
    }

    /// Convert from world coordinates to local coordinates
    inline glm::vec3 toLocal(const glm::vec3 &v) const {
        return glm::vec3(
            glm::dot(v, s), glm::dot(v, t), glm::dot(v, n)
        );
    }

    /// Convert from local coordinates to world coordinates
    inline glm::vec3 toWorld(const glm::vec3 &v) const {
        return s * v.x + t * v.y + n * v.z;
    }

    /** \brief Assuming that the given direction is in the local coordinate 
     * system, return the cosine of the angle between the normal and v */
    inline static float cosTheta(const glm::vec3 &v) {
        return v.z;
    }

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the sine of the angle between the normal and v */
    inline static float sinTheta(const glm::vec3 &v) {
        float temp = sinTheta2(v);
        if (temp <= 0.0f)
            return 0.0f;
        return std::sqrt(temp);
    }

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the tangent of the angle between the normal and v */
    inline static float tanTheta(const glm::vec3 &v) {
        float temp = 1 - v.z * v.z;
        if (temp <= 0.0f)
            return 0.0f;
        return std::sqrt(temp) / v.z;
    }

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the squared sine of the angle between the normal and v */
    inline static float sinTheta2(const glm::vec3 &v) {
        return 1.0f - v.z * v.z;
    }

    /** \brief Assuming that the given direction is in the local coordinate 
     * system, return the sine of the phi parameter in spherical coordinates */
    inline static float sinPhi(const glm::vec3 &v) {
        float sinTheta = Frame::sinTheta(v);
        if (sinTheta == 0.0f)
            return 1.0f;
        return math::clamp(v.y / sinTheta, -1.0f, 1.0f);
    }

    /** \brief Assuming that the given direction is in the local coordinate 
     * system, return the cosine of the phi parameter in spherical coordinates */
    inline static float cosPhi(const glm::vec3 &v) {
        float sinTheta = Frame::sinTheta(v);
        if (sinTheta == 0.0f)
            return 1.0f;
        return math::clamp(v.x / sinTheta, -1.0f, 1.0f);
    }

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the squared sine of the phi parameter in  spherical
     * coordinates */
    inline static float sinPhi2(const glm::vec3 &v) {
        return math::clamp(v.y * v.y / sinTheta2(v), 0.0f, 1.0f);
    }

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the squared cosine of the phi parameter in  spherical
     * coordinates */
    inline static float cosPhi2(const glm::vec3 &v) {
        return math::clamp(v.x * v.x / sinTheta2(v), 0.0f, 1.0f);
    }

    /// Equality test
    inline bool operator==(const Frame &frame) const {
        return frame.s == s && frame.t == t && frame.n == n;
    }

    /// Inequality test
    inline bool operator!=(const Frame &frame) const {
        return !operator==(frame);
    }

private:

    void coordinateSystem(const glm::vec3 &a, glm::vec3 &b, glm::vec3 &c) {
        if (std::abs(a.x) > std::abs(a.y)) {
            float invLen = 1.0f / std::sqrt(a.x * a.x + a.z * a.z);
            c = glm::vec3(a.z * invLen, 0.0f, -a.x * invLen);
        } else {
            float invLen = 1.0f / std::sqrt(a.y * a.y + a.z * a.z);
            c = glm::vec3(0.0f, a.z * invLen, -a.y * invLen);
        }
        b = glm::cross(c, a);
    }

};

}

#endif /* __FRAME_H */
