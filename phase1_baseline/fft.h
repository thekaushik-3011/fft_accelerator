#ifndef FFT_H
#define FFT_H

#include <complex>

#define FFT_LENGTH 1024
#define FFT_STAGES 10

typedef std::complex<float> complex_t;

void fft(complex_t x[FFT_LENGTH], const complex_t twiddles[FFT_LENGTH/2]);
void init_twiddles(complex_t twiddles[FFT_LENGTH/2]);

#endif
