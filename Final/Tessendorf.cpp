//
// Created by William Ma on 5/5/22.
//

#include "Tessendorf.h"

#define _USE_MATH_DEFINES
#include <cmath>


#define M_IMAG complex<float>(0.0f, 1.0f)

namespace tessendorf {

    using namespace pocketfft::detail;
    using namespace std;

    float phillips_spectrum(glm::vec2 vec_k, config config) {
        float k = glm::length(vec_k);
        if (fabs(k) < 1e-7) {
            return 0;
        }

        float k_squared = k * k;
        float t1 = -1.0f / (k_squared * config.max_wave_height() * config.max_wave_height());
        float t2 = exp(t1) / (k_squared * k_squared);
        float t2_2 = glm::dot(glm::normalize(vec_k), config.wind_dir);
        float t3 = config.spectrum_scale * t2 * (t2_2 * t2_2);
        return t3 * expf(-k_squared * config.min_wave_height() * config.min_wave_height());
    }

    glm::vec2 vec_k(glm::ivec2 vec_i, config config) {
        return 2.0f * (float) M_PI * glm::vec2(vec_i) / config.patch_size;
    }

    complex<float> fourier_amplitude_initial(array2d<complex<float>> iv, glm::ivec2 vec_i, config config) {
        complex<float> xi = iv.get((vec_i.x + iv.size_x) % iv.size_x, (vec_i.y + iv.size_y) % iv.size_y);
        float phillips = phillips_spectrum(vec_k(vec_i, config), config);
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

    float dispersion_relation(float k, config config) {
        float omega_k = sqrt(9.81f * k);
        float omega_0 = 2.0f * M_PI / config.period;
        return floor(omega_k / omega_0) * omega_0;
    }

    complex<float> fourier_amplitude(const array2d<complex<float>> &iv, glm::ivec2 vec_i, float t, config config) {
        float k = length(vec_k(vec_i, config));
        complex<float> e = exp(M_IMAG * dispersion_relation(k, config) * t);
        complex<float> a = fourier_amplitude_initial(iv, vec_i, config) * e;
        complex<float> b = conj(fourier_amplitude_initial(iv, -vec_i, config)) * conj(e);
        return a + b;
    }

    void fourier_amplitudes(array2d<complex<float>> out, const array2d<complex<float>> &iv, float t, config config) {
        for (int i = 0; i < iv.size_x; i++) {
            for (int j = 0; j < iv.size_y; j++) {
                glm::ivec2 vec_i(
                        (i + iv.size_x / 2) % iv.size_x - iv.size_x / 2,
                        (j + iv.size_y / 2) % iv.size_y - iv.size_y / 2
                );
                out.set(i, j, fourier_amplitude(iv, vec_i, t, config));
            }
        }
    }

    void gradient_amplitudes(
            array2d<complex<float>> out_x,
            array2d<complex<float>> out_y,
            array2d<complex<float>> fourier_amplitudes,
            config config
    ) {
        size_t size_x = out_x.size_x, size_y = out_x.size_y;

        assert(size_x == out_y.size_x);
        assert(size_y == out_y.size_y);

        assert(size_x == fourier_amplitudes.size_x);
        assert(size_y == fourier_amplitudes.size_y);

        for (int i = 0; i < size_x; i++) {
            for (int j = 0; j < size_y; j++) {
                glm::ivec2 vec_i(
                        (i + size_x / 2) % size_x - size_x / 2,
                        (j + size_y / 2) % size_y - size_y / 2
                );
                glm::vec2 k = vec_k(vec_i, config);
                complex<float> fa = fourier_amplitudes.get(i, j);
                out_x.set(i, j, M_IMAG * k.x * fa);
                out_y.set(i, j, M_IMAG * k.y * fa);
            }
        }
    }

    void ifft(array2d<float> out, const array2d<complex<float>> &fa, array2d<complex<float>> buffer, bool normalize) {
        size_t size_x = out.size_x, size_y = out.size_y;

        assert(size_x == fa.size_x);
        assert(size_y == fa.size_y);

        assert(size_x == buffer.size_x);
        assert(size_y == buffer.size_y);

        pocketfft::c2c(
                {size_x, size_y},
                {fa.stride_x, fa.stride_y},
                {buffer.stride_x, buffer.stride_y},
                {0, 1},
                BACKWARD,
                fa.data.get(),
                buffer.data.get(),
                normalize ? 1.0f / sqrt((float) (size_x * size_y)) : 1.0f,
                0
        );

        for (int i = 0; i < out.size_x; i++) {
            for (int j = 0; j < out.size_y; j++) {
                out.set(i, j, buffer.get(i, j).real());
            }
        }
    }

}
