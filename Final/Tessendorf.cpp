//
// Created by William Ma on 5/5/22.
//

#include "Tessendorf.h"

#ifdef __CYGWIN__
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#define PERIOD_T 10.0f
#define PATCH_SIZE glm::vec2(64, 64)
#define GRAVITY 9.81f
#define WIND_SPEED 12.0f
#define MAX_WAVE_HEIGHT (WIND_SPEED * WIND_SPEED / GRAVITY)
#define WIND_DIR glm::normalize(glm::vec2(0.2, 1))
#define MIN_WAVE_HEIGHT (1e-4f * MAX_WAVE_HEIGHT)
#define M_IMAG complex<float>(0.0f, 1.0f)
#define HEIGHT_SCALE (1.0f / PATCH_SIZE.x)

namespace tessendorf {

    using namespace pocketfft::detail;
    using namespace std;

    float phillips_spectrum(glm::vec2 vec_k) {
        float A = 3;
        float k = glm::length(vec_k);
        if (fabs(k) < 1e-7) {
            return 0;
        }

        float k_squared = k * k;
        float t1 = -1.0f / (k_squared * MAX_WAVE_HEIGHT * MAX_WAVE_HEIGHT);
        float t2 = exp(t1) / (k_squared * k_squared);
        float t2_2 = glm::dot(glm::normalize(vec_k), WIND_DIR);
        float t3 = A * t2 * (t2_2 * t2_2);
        return t3 * expf(-k_squared * MIN_WAVE_HEIGHT * MIN_WAVE_HEIGHT);
    }

    glm::vec2 vec_k(glm::ivec2 vec_i) {
        return 2.0f * (float) M_PI * glm::vec2(vec_i) / PATCH_SIZE;
    }

    complex<float> fourier_amplitude_initial(array2d<complex<float>> iv, glm::ivec2 vec_i) {
        complex<float> xi = iv.get((vec_i.x + iv.size_x) % iv.size_x, (vec_i.y + iv.size_y) % iv.size_y);
        float phillips = phillips_spectrum(vec_k(vec_i));
        return 1 / sqrt(2.0f) * xi * sqrt(phillips);
    }

    array2d<complex<float>> sample_initialization_vector(glm::ivec2 size, std::default_random_engine generator) {
        std::normal_distribution<float> normal{};

        array2d<complex<float>> iv(size.x, size.y);

        for (int i = 0; i < size.x; i++) {
            for (int j = 0; j < size.y; j++) {
                iv.set(i, j, complex<float>(normal(generator), normal(generator)));
            }
        }

        return iv;
    }

    array2d<complex<float>> test_initialization_vector(glm::ivec2 size) {
        array2d<complex<float>> iv(size.x, size.y);

        for (int i = 0; i < size.x; i++) {
            vector<complex<float>> row;
            row.reserve(size.y);

            for (int j = 0; j < size.y; j++) {
                iv.set(i, j, complex<float>(i * size.y + j, i * size.y + j));
            }
        }

        return iv;
    }

    float dispersion_relation(float k) {
        float omega_k = sqrt(GRAVITY * k);
        float omega_0 = 2.0f * M_PI / PERIOD_T;
        return floor(omega_k / omega_0) * omega_0;
    }

    complex<float> fourier_amplitude(const array2d<complex<float>> &iv, glm::ivec2 vec_i, float t) {
        float k = length(vec_k(vec_i));
        complex<float> e = exp(M_IMAG * dispersion_relation(k) * t);
        complex<float> a = fourier_amplitude_initial(iv, vec_i) * e;
        complex<float> b = conj(fourier_amplitude_initial(iv, -vec_i)) * conj(e);
        return a + b;
    }

    void fourier_amplitudes(array2d<complex<float>> out, const array2d<complex<float>> &iv, float t) {
        for (int i = 0; i < iv.size_x; i++) {
            for (int j = 0; j < iv.size_y; j++) {
                glm::ivec2 vec_i(
                        (i + iv.size_x / 2) % iv.size_x - iv.size_x / 2,
                        (j + iv.size_y / 2) % iv.size_y - iv.size_y / 2
                );
                out.set(i, j, fourier_amplitude(iv, vec_i, t));
            }
        }
    }

    void height_map(array2d<float> out, const array2d<complex<float>> &iv, float t) {
        array2d<complex<float>> fa(iv.size_x, iv.size_y);
        fourier_amplitudes(fa, iv, t);

        array2d<complex<float>> hf(iv.size_x, iv.size_y);
        pocketfft::c2c(
                {hf.size_x, hf.size_y},
                {fa.stride_x, fa.stride_y},
                {hf.stride_x, hf.stride_y},
                {0, 1},
                BACKWARD,
                fa.data.get(),
                hf.data.get(),
                HEIGHT_SCALE / 3.0f
        );

        for (int i = 0; i < out.size_x; i++) {
            for (int j = 0; j < out.size_y; j++) {
                out.set(i, j, hf.get(i, j).real());
            }
        }
    }

}
