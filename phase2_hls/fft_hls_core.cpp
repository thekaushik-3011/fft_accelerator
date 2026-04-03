#include "fft_hls.h"



// Single Decimation-In-Frequency SDF Stage
template<int STAGE>
void sdf_stage(hls::stream<complex_fixed_t>& in, hls::stream<complex_fixed_t>& out, const complex_fixed_t twiddles[FFT_LENGTH/2]) {
    constexpr int m = 1 << STAGE;
    constexpr int half_m = m / 2;
    constexpr int twiddle_step = FFT_LENGTH / m;
    
    // Shift register mapped optimally to RAM
    static complex_fixed_t sr[half_m];
    #pragma HLS bind_storage variable=sr type=ram_2p
    #pragma HLS dependence variable=sr type=inter false

    for (int c = 0; c < 2 * FFT_LENGTH - 1; c++) {
        #pragma HLS pipeline II=1
        complex_fixed_t val_in = in.read();
        
        bool second_half = (c & half_m) != 0;
        unsigned int index = c & (half_m - 1);
        
        complex_fixed_t val_sr = sr[index];
        complex_fixed_t val_out;
        
        if (!second_half) { 
            // 1st half: delay input, output passed data from previous state
            val_out = val_sr;
            sr[index] = val_in;
        } else { 
            // 2nd half: perform butterfly math
            complex_fixed_t w = twiddles[index * twiddle_step];
            val_out = val_sr + val_in; // T1
            complex_fixed_t diff = val_sr - val_in;
            data_t t2_real = diff.real() * w.real() - diff.imag() * w.imag();
            data_t t2_imag = diff.real() * w.imag() + diff.imag() * w.real();
            sr[index] = complex_fixed_t(t2_real, t2_imag);
        }
        
        out.write(val_out);
    }
}

// The complete continuous architecture
void fft_core_sdf(hls::stream<complex_fixed_t>& in_stream, hls::stream<complex_fixed_t>& out_stream, const complex_fixed_t twiddles[FFT_LENGTH/2]) {
    #pragma HLS dataflow
    
    hls::stream<complex_fixed_t> strm[FFT_STAGES];
    #pragma HLS stream variable=strm depth=2
    
    sdf_stage<10>(in_stream, strm[0], twiddles);
    sdf_stage<9>(strm[0], strm[1], twiddles);
    sdf_stage<8>(strm[1], strm[2], twiddles);
    sdf_stage<7>(strm[2], strm[3], twiddles);
    sdf_stage<6>(strm[3], strm[4], twiddles);
    sdf_stage<5>(strm[4], strm[5], twiddles);
    sdf_stage<4>(strm[5], strm[6], twiddles);
    sdf_stage<3>(strm[6], strm[7], twiddles);
    sdf_stage<2>(strm[7], strm[8], twiddles);
    sdf_stage<1>(strm[8], out_stream, twiddles);
}
