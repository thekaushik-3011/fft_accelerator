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
        
        ap_uint<32> real_raw = val.data.range(31, 0);
        ap_uint<32> imag_raw = val.data.range(63, 32);
        
        data_t real_val; real_val.range() = real_raw;
        data_t imag_val; imag_val.range() = imag_raw;
        
        x[i] = complex_fixed_t(real_val, imag_val);
    }
    
    // Call the newly created optimized core function (with pragmas)
    fft_core(x, W);
    
    write_out: for (int i = 0; i < FFT_LENGTH; i++) {
        axis_t val;
        
        ap_uint<32> real_raw = x[i].real().range();
        ap_uint<32> imag_raw = x[i].imag().range();
        
        ap_uint<64> packed_data;
        packed_data.range(31,0) = real_raw;
        packed_data.range(63,32) = imag_raw;
        
        val.data = packed_data;
        val.keep = -1; // All 8 bytes valid (64-bit complex)
        val.strb = -1;
        val.last = (i == FFT_LENGTH - 1) ? 1 : 0;
        out.write(val);
    }
}
