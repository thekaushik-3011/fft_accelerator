#ifndef FFT_FIXED_H
#define FFT_FIXED_H

#include <complex>
// Include ap_fixed.h for Vitis HLS fixed-point types
#include <ap_fixed.h>

#define FFT_LENGTH 1024
#define FFT_STAGES 10

// Fixed-point type definition.
// For a 1024-point FFT without scaling, values grow by N=1024, so we need log2(1024)=10 extra integer bits.
// We'll use 32 total bits with 16 integer bits (covering [-32768, 32767]).
typedef ap_fixed<32, 16> data_t;
typedef std::complex<data_t> complex_fixed_t;

void fft_fixed(complex_fixed_t x[FFT_LENGTH], const complex_fixed_t twiddles[FFT_LENGTH/2]);
void init_twiddles_fixed(complex_fixed_t twiddles[FFT_LENGTH/2]);

#endif
