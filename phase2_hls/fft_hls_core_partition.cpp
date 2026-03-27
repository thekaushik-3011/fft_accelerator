#include "fft_hls.h"

// Phase 2 Solution 3: Partitioning & Partial Unrolling
void bit_reverse_partition(complex_fixed_t x[FFT_LENGTH]) {
    complex_fixed_t temp[FFT_LENGTH];
    for (unsigned int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        unsigned int reversed = 0;
        for (int b = 0; b < FFT_STAGES; b++) {
            if ((i & (1 << b)) != 0) {
                reversed |= (1 << (FFT_STAGES - 1 - b));
            }
        }
        temp[reversed] = x[i];
    }
    for (unsigned int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        x[i] = temp[i];
    }
}

void fft_core(complex_fixed_t x[FFT_LENGTH], const complex_fixed_t twiddles[FFT_LENGTH/2]) {
    // Array partition on x is applied at top level in fft_top
    bit_reverse_partition(x);
    for (int stage = 1; stage <= FFT_STAGES; stage++) {
        int m = 1 << stage;
        int half_m = m / 2;
        int twiddle_step = FFT_LENGTH / m;
        
        for (int k = 0; k < FFT_LENGTH; k += m) {
            for (int j = 0; j < half_m; j++) {
                #pragma HLS pipeline II=1
                #pragma HLS unroll factor=2
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
