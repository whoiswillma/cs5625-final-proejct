//
// Created by William Ma on 5/13/22.
//

#include "OceanAnimator.h"

OceanAnimator::OceanAnimator(const std::shared_ptr<OceanScene>& scene) :
    scene(scene),
    buffers(scene->gridSize.x, scene->gridSize.y),
    displacement("displacement", scene->gridSize.x, scene->gridSize.y),
    gradX("gradX", scene->gridSize.x, scene->gridSize.y),
    gradZ("gradZ", scene->gridSize.x, scene->gridSize.y) {
}

void OceanAnimator::updateOceanBuffers(double time) {
    tessendorf::fourier_amplitudes(
            buffers.fourierAmplitudes,
            scene->tessendorfIv,
            (float) time,
            scene->config);
    tessendorf::ifft(
            buffers.displacementMap,
            buffers.fourierAmplitudes,
            buffers.buffer,
            true);
    tessendorf::gradient_amplitudes(
            buffers.gradientXAmplitudes,
            buffers.gradientZAmplitudes,
            buffers.fourierAmplitudes,
            scene->config);
    tessendorf::ifft(
            buffers.gradXMap,
            buffers.gradientXAmplitudes,
            buffers.buffer,
            false);
    tessendorf::ifft(
            buffers.gradZMap,
            buffers.gradientZAmplitudes,
            buffers.buffer,
            false);
}

OceanTextureBuffer::OceanTextureBuffer(
        std::string name,
        size_t size_x,
        size_t size_y
) : buffer(size_x, size_y),
    a(1),
    b(0),
    texture(std::make_shared<GLWrap::Texture2D>(
        glm::ivec2(size_x, size_y),
        GL_DEPTH_COMPONENT32F,
        GL_DEPTH_COMPONENT
    )) {
    texture->parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture->parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    texture->parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void OceanTextureBuffer::store(const tessendorf::array2d<float>& data) {
    buffer.copyFrom(data);

    float min = buffer.min();
    float max = buffer.max();
    buffer.plus(-min);
    buffer.times(1 / (max - min));
    a = max - min;
    b = min;

    glBindTexture(GL_TEXTURE_2D, texture->id());
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT32F,
            buffer.size_x,
            buffer.size_y,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            buffer.data.get()
    );
}

void OceanTextureBuffer::bindTextureAndUniforms(
        const std::string& name,
        const std::shared_ptr<GLWrap::Program>& program,
        int textureUnit
) {
    texture->bindToTextureUnit(textureUnit);
    program->uniform(name + "Map", textureUnit);
    program->uniform(name + "A", a);
    program->uniform(name + "B", b);
}

OceanBuffers::OceanBuffers(
        size_t x,
        size_t y
) : buffer(x, y),
    fourierAmplitudes(x, y),
    gradientXAmplitudes(x, y),
    gradientZAmplitudes(x, y),
    displacementMap(x, y),
    gradXMap(x, y),
    gradZMap(x, y) {
}
