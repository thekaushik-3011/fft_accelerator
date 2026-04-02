#include "fft_hls.h"
#include "twiddles.h" // Precomputed ROM for twiddle factors

// Input mapper: formats AXI Stream into strictly math stream
void input_stage(hls::stream<axis_t>& axis_in, hls::stream<complex_fixed_t>& sdf_in) {
    for (int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        axis_t val = axis_in.read();
        
        ap_uint<32> real_raw = val.data.range(31, 0);
        ap_uint<32> imag_raw = val.data.range(63, 32);
        
        data_t real_val; real_val.range() = real_raw;
        data_t imag_val; imag_val.range() = imag_raw;
        
        sdf_in.write(complex_fixed_t(real_val, imag_val));
    }
    
    // Flush the SDF pipeline dynamically without exposing state to Python
    for (int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        sdf_in.write(complex_fixed_t(0, 0));
    }
}

// Output mapper: sorts the bit-reversed stream naturally via 1 Ping-Pong Buffer
// and formats it back to AXI-Stream metadata
void output_reorder_stage(hls::stream<complex_fixed_t>& sdf_out, hls::stream<axis_t>& axis_out) {
    complex_fixed_t frame_buf[FFT_LENGTH];
    #pragma HLS bind_storage variable=frame_buf type=ram_2p
    
    // SDF outputs 1024 dummy/garbage outputs followed by the 1024 valid bit-reversed outputs
    for (int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        sdf_out.read(); // Discard cycle
    }
    
    // Read the actual bit-reversed stream into the frame buffer naturally
    for (int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        
        unsigned int reversed = 0;
        // Letting HLS unroll this completely synthesizes a combinatorial index map (ROM)
        for (int b = 0; b < FFT_STAGES; b++) {
            #pragma HLS unroll
            if ((i & (1 << b)) != 0) {
                reversed |= (1 << (FFT_STAGES - 1 - b));
            }
        }
        frame_buf[reversed] = sdf_out.read();
    }
    
    // Write out the cleanly sorted array
    for (int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        axis_t val;
        
        ap_uint<32> real_raw = frame_buf[i].real().range();
        ap_uint<32> imag_raw = frame_buf[i].imag().range();
        
        ap_uint<64> packed_data;
        packed_data.range(31,0) = real_raw;
        packed_data.range(63,32) = imag_raw;
        
        val.data = packed_data;
        val.keep = -1; 
        val.strb = -1;
        val.last = (i == FFT_LENGTH - 1) ? 1 : 0;
        axis_out.write(val);
    }
}

void fft_top(hls::stream<axis_t>& in, hls::stream<axis_t>& out) {
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL
    
    #pragma HLS dataflow

    // Internal connecting streams
    hls::stream<complex_fixed_t> sdf_in("sdf_in");
    hls::stream<complex_fixed_t> sdf_out("sdf_out");
    #pragma HLS stream variable=sdf_in depth=2
    #pragma HLS stream variable=sdf_out depth=2

    // The entire multi-stage streaming architecture
    input_stage(in, sdf_in);
    fft_core_sdf(sdf_in, sdf_out, W);
    output_reorder_stage(sdf_out, out);
}
