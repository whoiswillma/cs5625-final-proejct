//
// Created by William Ma on 5/5/22.
//

#ifndef CS5625_TESSENDORF_H
#define CS5625_TESSENDORF_H

#include <iostream>
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include "pocketfft_hdronly.h"

namespace tessendorf {

    using namespace pocketfft::detail;
    using namespace std;

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

        float min() {
            float min = data[0];
            for (int i = 0; i < size_x; i++) {
                for (int j = 0; j < size_y; j++) {
                    min = fmin(min, data[size_y * i + j]);
                }
            }
            return min;
        }

        float max() {
            float max = data[0];
            for (int i = 0; i < size_x; i++) {
                for (int j = 0; j < size_y; j++) {
                    max = fmax(max, data[size_y * i + j]);
                }
            }
            return max;
        }

        void times(float x) {
            for (int i = 0; i < size_x; i++) {
                for (int j = 0; j < size_y; j++) {
                    data[size_y * i + j] *= x;
                }
            }
        }

        void plus(float x) {
            for (int i = 0; i < size_x; i++) {
                for (int j = 0; j < size_y; j++) {
                    data[size_y * i + j] += x;
                }
            }
        }

        void dump() {
            for (int i = 0; i < size_x; i++) {
                for (int j = 0; j < size_y; j++) {
                    cout << data[size_y * i + j] << " ";
                }
                cout << endl;
            }
        }

    };

    array2d<complex<float>> sample_initialization_vector(glm::ivec2 size, std::default_random_engine generator);

    array2d<complex<float>> test_initialization_vector(glm::ivec2 size);

    void height_map(array2d<float> out, const array2d<complex<float>> &iv, float t);
}

#endif //CS5625_TESSENDORF_H
