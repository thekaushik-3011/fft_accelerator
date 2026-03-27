#include "fft.h"
#include <iostream>
#include <cmath>
#include <chrono>

int main() {
    complex_t x[FFT_LENGTH];
    complex_t twiddles[FFT_LENGTH/2];
    
    init_twiddles(twiddles);
    
    // Generate test signal: sum of 2 sine waves exactly on bins 10 and 50
    for (int i = 0; i < FFT_LENGTH; i++) {
        float bin1 = 10.0;
        float bin2 = 50.0;
        x[i] = complex_t(std::sin(2.0 * M_PI * bin1 * i / FFT_LENGTH) + 
                         0.5 * std::sin(2.0 * M_PI * bin2 * i / FFT_LENGTH), 0);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    fft(x, twiddles);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::micro> elapsed = end - start;
    std::cout << "--- Floating-Point FFT ---" << std::endl;
    std::cout << "Execution Time: " << elapsed.count() << " us" << std::endl;
    
    // Check magnitudes
    std::cout << "Bin 10 Magnitude: " << std::abs(x[10]) << " (Expected ~512)" << std::endl;
    std::cout << "Bin 50 Magnitude: " << std::abs(x[50]) << " (Expected ~256)" << std::endl;
    
    return 0;
}
