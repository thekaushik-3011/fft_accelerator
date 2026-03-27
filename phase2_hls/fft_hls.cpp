#include "fft_hls.h"
#include "twiddles.h" // Precomputed ROM for twiddle factors

// Declare our local modified core function
void fft_core(complex_fixed_t x[FFT_LENGTH], const complex_fixed_t twiddles[FFT_LENGTH/2]);

void fft_top(hls::stream<axis_t>& in, hls::stream<axis_t>& out) {
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    complex_fixed_t x[FFT_LENGTH];
    
    // Must propagate the partition to the top level BRAM instantiation
    #pragma HLS array_partition variable=x type=cyclic factor=2 dim=1

    read_in: for (int i = 0; i < FFT_LENGTH; i++) {
        axis_t val = in.read();
        x[i] = val.data;
    }
    
    // Call the newly created optimized core function (with pragmas)
    fft_core(x, W);
    
    write_out: for (int i = 0; i < FFT_LENGTH; i++) {
        axis_t val;
        val.data = x[i];
        val.last = (i == FFT_LENGTH - 1);
        out.write(val);
    }
}
