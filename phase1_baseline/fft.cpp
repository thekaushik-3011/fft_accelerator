#include "fft.h"
#include <cmath>
#include <iostream>

void bit_reverse(complex_t x[FFT_LENGTH]) {
    unsigned int j = 0;
    for (unsigned int i = 0; i < FFT_LENGTH - 1; i++) {
        if (i < j) {
            complex_t temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
        unsigned int m = FFT_LENGTH / 2;
        while (m <= j) {
            j -= m;
            m /= 2;
        }
        j += m;
    }
}

void init_twiddles(complex_t twiddles[FFT_LENGTH/2]) {
    for (int i = 0; i < FFT_LENGTH/2; i++) {
        twiddles[i] = std::polar(1.0f, -2.0f * (float)M_PI * i / FFT_LENGTH);
    }
}

void fft(complex_t x[FFT_LENGTH], const complex_t twiddles[FFT_LENGTH/2]) {
    bit_reverse(x);
    for (int stage = 1; stage <= FFT_STAGES; stage++) {
        int m = 1 << stage;
        int half_m = m / 2;
        int twiddle_step = FFT_LENGTH / m;
        
        for (int k = 0; k < FFT_LENGTH; k += m) {
            for (int j = 0; j < half_m; j++) {
                complex_t w = twiddles[j * twiddle_step];
                complex_t t = w * x[k + j + half_m];
                complex_t u = x[k + j];
                x[k + j] = u + t;
                x[k + j + half_m] = u - t;
            }
        }
    }
}
