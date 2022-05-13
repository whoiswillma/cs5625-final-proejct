//
// Created by William Ma on 5/13/22.
//

#ifndef CS5625_OCEANANIMATOR_H
#define CS5625_OCEANANIMATOR_H

#include <memory>
#include <utility>
#include "OceanScene.h"
#include "GLWrap/Texture2D.hpp"
#include "GLWrap/Program.hpp"

class OceanTextureBuffer {
    tessendorf::array2d<float> buffer;
    float a, b;
    std::shared_ptr<GLWrap::Texture2D> texture;

public:
    OceanTextureBuffer(std::string name, size_t x, size_t y);
    void store(const tessendorf::array2d<float>& data);
    void bindTextureAndUniforms(
            const std::string& name,
            const std::shared_ptr<GLWrap::Program> &program,
            int textureUnit
    );
};

struct OceanBuffers {
    tessendorf::array2d<std::complex<float>> buffer;
    tessendorf::array2d<std::complex<float>> fourierAmplitudes;
    tessendorf::array2d<std::complex<float>> gradientXAmplitudes;
    tessendorf::array2d<std::complex<float>> gradientZAmplitudes;
    tessendorf::array2d<float> displacementMap;
    tessendorf::array2d<float> gradXMap;
    tessendorf::array2d<float> gradZMap;

    OceanBuffers(size_t x, size_t y);
};

struct OceanAnimator {
    OceanTextureBuffer displacement;
    OceanTextureBuffer gradX;
    OceanTextureBuffer gradZ;

    explicit OceanAnimator(const std::shared_ptr<OceanScene>& scene);

    void updateOceanBuffers(double time);

    OceanBuffers buffers;

private:
    std::shared_ptr<OceanScene> scene;
};


#endif //CS5625_OCEANANIMATOR_H
