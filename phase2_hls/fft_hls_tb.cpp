#include "fft_hls.h"
#include <iostream>
#include <cmath>

int main() {
    hls::stream<axis_t> in_stream("in_stream");
    hls::stream<axis_t> out_stream("out_stream");
    
    // Generate test signal: sum of 2 sine waves
    for (int i = 0; i < FFT_LENGTH; i++) {
        float bin1 = 10.0;
        float bin2 = 50.0;
        float val = std::sin(2.0 * M_PI * bin1 * i / FFT_LENGTH) + 
                    0.5 * std::sin(2.0 * M_PI * bin2 * i / FFT_LENGTH);
        
        ap_uint<32> real_raw = data_t(val).range();
        ap_uint<32> imag_raw = data_t(0.0).range();
        
        ap_uint<64> packed_data;
        packed_data.range(31,0) = real_raw;
        packed_data.range(63,32) = imag_raw;
        
        axis_t in_val;
        in_val.data = packed_data;
        in_val.keep = -1;
        in_val.strb = -1;
        in_val.last = (i == FFT_LENGTH - 1) ? 1 : 0;
        in_stream.write(in_val);
    }
    
    // Run the top-level HLS function
    fft_top(in_stream, out_stream);
    
    // Read results from stream
    complex_fixed_t out[FFT_LENGTH];
    bool last_signal_correct = false;
    for (int i = 0; i < FFT_LENGTH; i++) {
        axis_t out_val = out_stream.read();
        ap_uint<32> real_raw = out_val.data.range(31,0);
        ap_uint<32> imag_raw = out_val.data.range(63,32);
        
        data_t real_val; real_val.range() = real_raw;
        data_t imag_val; imag_val.range() = imag_raw;
        
        out[i] = complex_fixed_t(real_val, imag_val);
        if (i == FFT_LENGTH - 1 && out_val.last == 1) {
            last_signal_correct = true;
        }
    }
    
    // Check results
    std::cout << "--- HLS C-Simulation (Fixed-Point) ---" << std::endl;
    // Calculate the complex magnitude: sqrt(real^2 + imag^2)
    float mag10 = std::hypot((float)out[10].real(), (float)out[10].imag());
    float mag50 = std::hypot((float)out[50].real(), (float)out[50].imag());
    
    std::cout << "Bin 10 Magnitude: " << mag10 << " (Expected ~512)" << std::endl;
    std::cout << "Bin 50 Magnitude: " << mag50 << " (Expected ~256)" << std::endl;
    std::cout << "TLAST Signal correctly asserted: " << (last_signal_correct ? "YES" : "NO") << std::endl;
    
    // Tight check: must be within 10% of the mathematically correct magnitudes
    bool mag10_ok = (mag10 > 460 && mag10 < 564);  // 512 ± 10%
    bool mag50_ok = (mag50 > 230 && mag50 < 282);  // 256 ± 10%
    
    if (mag10_ok && mag50_ok && last_signal_correct) {
        std::cout << "C-SIM TEST PASSED" << std::endl;
        return 0;
    } else {
        if (!mag10_ok) std::cout << "FAIL: Bin 10 magnitude out of range (got " << mag10 << ", expected 460-564)" << std::endl;
        if (!mag50_ok) std::cout << "FAIL: Bin 50 magnitude out of range (got " << mag50 << ", expected 230-282)" << std::endl;
        if (!last_signal_correct) std::cout << "FAIL: TLAST not asserted" << std::endl;
        std::cout << "C-SIM TEST FAILED" << std::endl;
        return 1;
    }
}
