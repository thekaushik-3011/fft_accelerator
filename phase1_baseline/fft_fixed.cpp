#include "fft_fixed.h"
#include <cmath>
#include <algorithm>

void bit_reverse_fixed(complex_fixed_t x[FFT_LENGTH]) {
    unsigned int j = 0;
    for (unsigned int i = 0; i < FFT_LENGTH - 1; i++) {
        if (i < j) {
            std::swap(x[i], x[j]);
        }
        unsigned int m = FFT_LENGTH / 2;
        while (m <= j) {
            j -= m;
            m /= 2;
        }
        j += m;
    }
}

void init_twiddles_fixed(complex_fixed_t twiddles[FFT_LENGTH/2]) {
    for (int i = 0; i < FFT_LENGTH/2; i++) {
        float real = std::cos(-2.0f * (float)M_PI * i / FFT_LENGTH);
        float imag = std::sin(-2.0f * (float)M_PI * i / FFT_LENGTH);
        twiddles[i] = complex_fixed_t(data_t(real), data_t(imag));
    }
}

void fft_fixed(complex_fixed_t x[FFT_LENGTH], const complex_fixed_t twiddles[FFT_LENGTH/2]) {
    bit_reverse_fixed(x);
    for (int stage = 1; stage <= FFT_STAGES; stage++) {
        int m = 1 << stage;
        int half_m = m / 2;
        int twiddle_step = FFT_LENGTH / m;
        
        for (int k = 0; k < FFT_LENGTH; k += m) {
            for (int j = 0; j < half_m; j++) {
                complex_fixed_t w = twiddles[j * twiddle_step];
                complex_fixed_t u = x[k + j];
                
                // Using explicit arithmetic because complex multiplication
                // with ap_fixed can sometimes be tricky depending on the library version
                data_t t_real = w.real() * x[k + j + half_m].real() - w.imag() * x[k + j + half_m].imag();
                data_t t_imag = w.real() * x[k + j + half_m].imag() + w.imag() * x[k + j + half_m].real();
                complex_fixed_t t(t_real, t_imag);
                
                x[k + j] = u + t;
                x[k + j + half_m] = u - t;
            }
        }
    }
}
