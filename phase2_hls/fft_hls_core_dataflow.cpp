#include "fft_hls.h"

// Static lookup for bit reversal to avoid loops inside dataflow
void bit_reverse_dataflow(complex_fixed_t in[FFT_LENGTH], complex_fixed_t out[FFT_LENGTH]) {
    for (unsigned int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        unsigned int reversed = 0;
        for (int b = 0; b < FFT_STAGES; b++) {
            if ((i & (1 << b)) != 0) {
                reversed |= (1 << (FFT_STAGES - 1 - b));
            }
        }
        out[reversed] = in[i];
    }
}

// A single FFT stage hardware unit
void fft_stage(int stage, complex_fixed_t in[FFT_LENGTH], complex_fixed_t out[FFT_LENGTH], const complex_fixed_t twiddles[FFT_LENGTH/2]) {
    int m = 1 << stage;
    int half_m = m / 2;
    int twiddle_step = FFT_LENGTH / m;
    
    // In Dataflow, we read from 'in' and write to 'out' (Ping-Pong buffers)
    // to completely avoid read/write conflicts on the same array.
    BUTTERFLY_OUTER: for (int k = 0; k < FFT_LENGTH; k += m) {
        BUTTERFLY_INNER: for (int j = 0; j < half_m; j++) {
            #pragma HLS pipeline II=1
            
            complex_fixed_t w = twiddles[j * twiddle_step];
            complex_fixed_t u = in[k + j];
            complex_fixed_t v = in[k + j + half_m];
            
            data_t t_real = w.real() * v.real() - w.imag() * v.imag();
            data_t t_imag = w.real() * v.imag() + w.imag() * v.real();
            complex_fixed_t t(t_real, t_imag);
            
            // Write directly to the next stage's buffer
            out[k + j] = u + t;
            out[k + j + half_m] = u - t;
        }
    }
}

void fft_core(complex_fixed_t x[FFT_LENGTH], const complex_fixed_t twiddles[FFT_LENGTH/2]) {
    // PHASE 2 SOLUTION 4: DATAFLOW AND UNROLLING
    #pragma HLS dataflow
    
    // We create a separate 2D array matrix to hold the Ping-Pong buffers between hardware stages.
    complex_fixed_t stage_data[FFT_STAGES + 1][FFT_LENGTH];
    
    // Stage 0: Bit reversal
    bit_reverse_dataflow(x, stage_data[0]);
    
    // Instantiates 10 separate physical butterfly computation circuits
    UNROLL_STAGES: for (int stage = 1; stage <= FFT_STAGES; stage++) {
        #pragma HLS unroll
        fft_stage(stage, stage_data[stage - 1], stage_data[stage], twiddles);
    }
    
    // Stage 11: Write final output back to x 
    // (In reality this just wires the stream out)
    for (int i = 0; i < FFT_LENGTH; i++) {
        #pragma HLS pipeline II=1
        x[i] = stage_data[FFT_STAGES][i];
    }
}
