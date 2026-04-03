#pragma once
#include "fft_hls.h"

void fft_core_sdf(
    hls::stream<complex_fixed_t>& in_stream,
    hls::stream<complex_fixed_t>& out_stream,
    const complex_fixed_t twiddles[FFT_LENGTH/2]
);