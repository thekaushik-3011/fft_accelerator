#include "fft_hls.h"
#include <iostream>
#include <cmath>

int main() {
    complex_fixed_t in[FFT_LENGTH];
    complex_fixed_t out[FFT_LENGTH];
    
    // Generate test signal: sum of 2 sine waves
    for (int i = 0; i < FFT_LENGTH; i++) {
        float bin1 = 10.0;
        float bin2 = 50.0;
        float val = std::sin(2.0 * M_PI * bin1 * i / FFT_LENGTH) + 
                    0.5 * std::sin(2.0 * M_PI * bin2 * i / FFT_LENGTH);
        in[i] = complex_fixed_t(data_t(val), data_t(0.0));
    }
    
    // Run the top-level HLS function
    fft_top(in, out);
    
    // Check results
    std::cout << "--- HLS C-Simulation (Fixed-Point) ---" << std::endl;
    // Calculate the complex magnitude: sqrt(real^2 + imag^2)
    float mag10 = std::hypot((float)out[10].real(), (float)out[10].imag());
    float mag50 = std::hypot((float)out[50].real(), (float)out[50].imag());
    
    std::cout << "Bin 10 Magnitude: " << mag10 << " (Expected ~512)" << std::endl;
    std::cout << "Bin 50 Magnitude: " << mag50 << " (Expected ~256)" << std::endl;
    
    // Simple check on the complex magnitudes
    if (mag10 > 500 && mag50 > 250) {
        std::cout << "C-SIM TEST PASSED" << std::endl;
        return 0;
    } else {
        std::cout << "C-SIM TEST FAILED" << std::endl;
        return 1;
    }
}
