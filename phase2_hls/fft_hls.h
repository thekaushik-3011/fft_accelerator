#ifndef FFT_HLS_H
#define FFT_HLS_H

#include "../phase1_baseline/fft_fixed.h"

// Top-level function wrapper for HLS synthesis.
// We use AXI4-Stream interfaces for the in and out arrays.
void fft_top(complex_fixed_t in[FFT_LENGTH], complex_fixed_t out[FFT_LENGTH]);

#endif
