//
// Created by William Ma on 5/5/22.
//

#ifdef __CYGWIN__
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <iostream>
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include "pocketfft_hdronly.h"

#define PERIOD_T 10.0f
#define PATCH_SIZE glm::vec2(64, 64)
#define GRID_SIZE glm::ivec2(10, 10)
#define GRAVITY 9.81f
#define WIND_SPEED 12.0f
#define MAX_WAVE_HEIGHT (WIND_SPEED * WIND_SPEED / GRAVITY)
#define WIND_DIR glm::normalize(glm::vec2(0.2, 1))
#define MIN_WAVE_HEIGHT (1e-4f * MAX_WAVE_HEIGHT)
#define M_IMAG complex<float>(0.0f, 1.0f)
#define HEIGHT_SCALE (1.0f / 3.0f)

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

    template<typename T>
    struct array2d {
        size_t size_x, size_y;
        ptrdiff_t stride_x, stride_y;
        shared_ptr<T[]> data;

        array2d(size_t size_x, size_t size_y) :
                size_x(size_x),
                size_y(size_y),
                stride_x(sizeof(T) * size_y),
                stride_y(sizeof(T)),
                data((T *) malloc(sizeof(T) * size_x * size_y)) {
        }

        T get(size_t x, size_t y) {
            return data[x * size_y + y];
        }

        void set(size_t x, size_t y, T v) {
            data[x * size_y + y] = v;
        }

    };

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

    void gradient_amplitudes(
            array2d<complex<float>> out_x,
            array2d<complex<float>> out_y,
            array2d<complex<float>> fourier_amplitudes
    ) {
        for (int i = 0; i < fourier_amplitudes.size_x; i++) {
            for (int j = 0; j < fourier_amplitudes.size_y; j++) {
                glm::ivec2 vec_i(
                        (i + fourier_amplitudes.size_x / 2) % fourier_amplitudes.size_x - fourier_amplitudes.size_x / 2,
                        (j + fourier_amplitudes.size_y / 2) % fourier_amplitudes.size_y - fourier_amplitudes.size_y / 2
                );
                glm::vec2 k = vec_k(vec_i);
                complex<float> fa = fourier_amplitudes.get(i, j);
                out_x.set(i, j, M_IMAG * k.x * fa);
                out_y.set(i, j, M_IMAG * k.y * fa);
            }
        }
    }

}

using namespace tessendorf;

int main() {
    array2d<complex<float>> iv = test_initialization_vector(GRID_SIZE);

    array2d<complex<float>> in(GRID_SIZE.x, GRID_SIZE.y);

    for (int i = 0; i < GRID_SIZE.x; i++) {
        for (int j = 0; j < GRID_SIZE.y; j++) {
            in.set(i, j, cos(2 * M_PI * i / GRID_SIZE.x) + cos(2 * M_PI * j / GRID_SIZE.y));
        }
    }

    for (int i = 0; i < GRID_SIZE.x; i++) {
        for (int j = 0; j < GRID_SIZE.y; j++) {
            cout << in.get(i, j) << " ";
        }
        cout << endl;
    }

    cout << endl;

    array2d<complex<float>> out(GRID_SIZE.x, GRID_SIZE.y);
    pocketfft::c2c(
            {in.size_x, in.size_y},
            {in.stride_x, in.stride_y},
            {out.stride_x, out.stride_y},
            {0, 1},
            BACKWARD,
            in.data.get(),
            out.data.get(),
            1.0f / sqrt((float) GRID_SIZE.x * GRID_SIZE.y)
    );

    for (int i = 0; i < GRID_SIZE.x; i++) {
        for (int j = 0; j < GRID_SIZE.y; j++) {
            cout << out.get(i, j) << " ";
        }
        cout << endl;
    }

//    array2d<complex<float>> ga_x(GRID_SIZE.x, GRID_SIZE.y);
//    array2d<complex<float>> ga_y(GRID_SIZE.x, GRID_SIZE.y);
//    gradient_amplitudes(ga_x, ga_y, fa);
//
//    array2d<complex<float>> n_x(GRID_SIZE.x, GRID_SIZE.y);
//    array2d<complex<float>> n_y(GRID_SIZE.x, GRID_SIZE.y);
//
//    pocketfft::c2c(
//            {ga_x.size_x, ga_x.size_y},
//            {ga_x.stride_x, ga_x.stride_y},
//            {n_x.stride_x, n_x.stride_y},
//            {0, 1},
//            BACKWARD,
//            ga_x.data.get(),
//            n_x.data.get(),
//            1.0f
//    );
//
//    pocketfft::c2c(
//            {ga_y.size_x, ga_y.size_y},
//            {ga_y.stride_x, ga_y.stride_y},
//            {n_y.stride_x, n_y.stride_y},
//            {0, 1},
//            BACKWARD,
//            ga_y.data.get(),
//            n_y.data.get(),
//            1.0f
//    );
//
//    for (int i = 0; i < GRID_SIZE.x; i++) {
//        for (int j = 0; j < GRID_SIZE.y; j++) {
//            std::cout << "(" << n_x.get(i, j) << ", " << n_y.get(i, j) << ")" << " ";
//        }
//        std::cout << std::endl;
//    }

//    for (int i = 0; i < 1000; i++) {
//        array2d<complex<float>> fa(GRID_SIZE.x, GRID_SIZE.y);
//        fourier_amplitudes(fa, iv, i);
//
//        array2d<complex<float>> hf(GRID_SIZE.x, GRID_SIZE.y);
//        pocketfft::c2c(
//                {hf.size_x, hf.size_y},
//                {fa.stride_x, fa.stride_y},
//                {hf.stride_x, hf.stride_y},
//                {0, 1},
//                BACKWARD,
//                fa.data.get(),
//                hf.data.get(),
//                HEIGHT_SCALE
//        );
//
//        array2d<complex<float>> ga_x(GRID_SIZE.x, GRID_SIZE.y);
//        array2d<complex<float>> ga_y(GRID_SIZE.x, GRID_SIZE.y);
//        gradient_amplitudes(ga_x, ga_y, fa, i);
//
//        array2d<complex<float>> n_x(GRID_SIZE.x, GRID_SIZE.y);
//        array2d<complex<float>> n_y(GRID_SIZE.x, GRID_SIZE.y);
//
//        pocketfft::c2c(
//                {ga_x.size_x, ga_x.size_y},
//                {ga_x.stride_x, ga_x.stride_y},
//                {n_x.stride_x, n_x.stride_y},
//                {0, 1},
//                BACKWARD,
//                ga_x.data.get(),
//                n_x.data.get(),
//                1.0f
//        );
//
//        pocketfft::c2c(
//                {ga_y.size_x, ga_y.size_y},
//                {ga_y.stride_x, ga_y.stride_y},
//                {n_y.stride_x, n_y.stride_y},
//                {0, 1},
//                BACKWARD,
//                ga_x.data.get(),
//                n_y.data.get(),
//                1.0f
//        );
//
//        array2d<complex<float>>
//    }
}
