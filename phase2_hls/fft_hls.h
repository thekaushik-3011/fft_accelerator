#ifndef FFT_HLS_H
#define FFT_HLS_H

#include "../phase1_baseline/fft_fixed.h"

#include <hls_stream.h>

#include <ap_int.h>

#include "ap_axi_sdata.h"

// Define AXI-Stream payload using native perfectly aligned Xilinx struct
typedef ap_axiu<64, 0, 0, 0> axis_t;

// Top-level function wrapper for HLS synthesis.
void fft_top(hls::stream<axis_t>& in, hls::stream<axis_t>& out);

#endif
