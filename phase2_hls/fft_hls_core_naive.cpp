#include "fft_hls.h"

// Phase 2 Solution 1: Naive (Direct Translation)
void bit_reverse_naive(complex_fixed_t x[FFT_LENGTH]) {
    // Standard in-place bit reversal without pragmas
    unsigned int j = 0;
    for (unsigned int i = 0; i < FFT_LENGTH - 1; i++) {
        if (i < j) {
            complex_fixed_t temp = x[i];
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

void fft_core(complex_fixed_t x[FFT_LENGTH], const complex_fixed_t twiddles[FFT_LENGTH/2]) {
    bit_reverse_naive(x);
    for (int stage = 1; stage <= FFT_STAGES; stage++) {
        int m = 1 << stage;
        int half_m = m / 2;
        int twiddle_step = FFT_LENGTH / m;
        
        for (int k = 0; k < FFT_LENGTH; k += m) {
            for (int j = 0; j < half_m; j++) {
                complex_fixed_t w = twiddles[j * twiddle_step];
                complex_fixed_t u = x[k + j];
                complex_fixed_t v = x[k + j + half_m];
                
                data_t t_real = w.real() * v.real() - w.imag() * v.imag();
                data_t t_imag = w.real() * v.imag() + w.imag() * v.real();
                complex_fixed_t t(t_real, t_imag);
                
                x[k + j] = u + t;
                x[k + j + half_m] = u - t;
            }
        }
    }
}
