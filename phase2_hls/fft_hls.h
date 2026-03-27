#ifndef FFT_HLS_H
#define FFT_HLS_H

#include "../phase1_baseline/fft_fixed.h"

#include <hls_stream.h>

// Define AXI-Stream payload
struct axis_t {
    complex_fixed_t data;
    bool last;
};

// Top-level function wrapper for HLS synthesis.
void fft_top(hls::stream<axis_t>& in, hls::stream<axis_t>& out);

#endif
