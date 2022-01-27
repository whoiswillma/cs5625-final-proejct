// Sky.hpp
//
// Cornell CS5625 Spring 2020
//
// author: Steve Marschner, April 2020
//

#pragma once

#include "common.hpp"

#include <glm/glm.hpp>

#include <GLWrap/Program.hpp>

namespace RTUtil {

    // A simple implementation of Preetham sky model that works with
    // the fragment shader code in sunsky.fs to evaluate sun and sky
    // radiance based on sun position and atmospheric conditions.
    //
    // To use it, link sunsky.fs into a shader program, create a
    // Sky instance with the parameters you want, and call
    // setUniforms with the shader program before you draw.  Then
    // in the fragment shader code you can call
    //     sunskyRadiance(worldDir)
    // to get an RGB color for the sky in a given world-space
    // direction.

    class RTUTIL_EXPORT Sky {
    public:

        Sky(float theta_sun, float turbidity)
        : theta_sun(theta_sun), turbidity(turbidity) { 
            init();
        }

        void setThetaSun(float theta_sun) {
            this->theta_sun = theta_sun;
        }

        void setTurbidity(float turbidity) {
            this->turbidity = turbidity;
        }

        // Sets the uniforms that are required for 
        // the sunskyRadiance shader function to operate.
        void setUniforms(GLWrap::Program &);

    private:
        // glm doesn't have a mat2x5/vec5 so we make our own types
        class vec5 {
            public: 
                float x, y, z, w, v;

                vec5(
                    float x, float y, float z, float w, float v
                ) : x(x), y(y), z(z), w(w), v(v) {}

                vec5() : x(0), y(0), z(0), w(0), v(0) {}

                vec5& operator+=(vec5 other) {
                    x += other.x;
                    y += other.y;
                    z += other.z;
                    w += other.w;
                    v += other.v;

                    return *this;
                }

                vec5& operator*=(float other) {
                    x *= other;
                    y *= other;
                    z *= other;
                    w *= other;
                    v *= other;

                    return *this;
                }

                friend vec5 operator+(vec5 a, vec5 b) {
                    return a += b;
                }

                friend vec5 operator*(vec5 a, float b) {
                    return a *= b;
                }
        };

        class mat2x5 {
            vec5 x, y;
            public: 
                mat2x5(vec5 x, vec5 y) : x(x), y(y) {}
                mat2x5() : x(), y() {}

                friend vec5 operator*(mat2x5 m, glm::vec2 v) {
                    return m.x * v.x + m.y * v.y; 
                }
        };

        float theta_sun, turbidity;

        // Matrices from the appendix of the paper
        // that are needed to compute the parameters to
        // the sky model.

        mat2x5 cY, cx, cy; 
        glm::mat4x3 Mx, My;

        void init();

    };

}

