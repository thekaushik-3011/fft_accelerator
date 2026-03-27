#include "fft.h"
#include "fft_fixed.h"
#include <iostream>
#include <cmath>

int main() {
    complex_t x_float[FFT_LENGTH];
    complex_fixed_t x_fixed[FFT_LENGTH];
    complex_t twiddles_float[FFT_LENGTH/2];
    complex_fixed_t twiddles_fixed[FFT_LENGTH/2];
    
    init_twiddles(twiddles_float);
    init_twiddles_fixed(twiddles_fixed);
    
    // Test signal: two sine waves
    for (int i = 0; i < FFT_LENGTH; i++) {
        float bin1 = 10.0;
        float bin2 = 50.0;
        float val = std::sin(2.0 * M_PI * bin1 * i / FFT_LENGTH) + 
                    0.5 * std::sin(2.0 * M_PI * bin2 * i / FFT_LENGTH);
        
        x_float[i] = complex_t(val, 0.0);
        x_fixed[i] = complex_fixed_t(data_t(val), data_t(0.0));
    }
    
    fft(x_float, twiddles_float);
    fft_fixed(x_fixed, twiddles_fixed);
    
    double signal_power = 0.0;
    double noise_power = 0.0;
    
    for (int i = 0; i < FFT_LENGTH; i++) {
        double real_fl = x_float[i].real();
        double imag_fl = x_float[i].imag();
        double real_fx = (double)x_fixed[i].real();
        double imag_fx = (double)x_fixed[i].imag();
        
        double diff_real = real_fl - real_fx;
        double diff_imag = imag_fl - imag_fx;
        
        signal_power += (real_fl * real_fl) + (imag_fl * imag_fl);
        noise_power += (diff_real * diff_real) + (diff_imag * diff_imag);
    }
    
    double snr = 10.0 * std::log10(signal_power / noise_power);
    
    std::cout << "--- Fixed-Point Q32.16 vs Floating-Point SNR ---" << std::endl;
    std::cout << "Signal Power: " << signal_power << std::endl;
    std::cout << "Noise Power:  " << noise_power << std::endl;
    std::cout << "SNR (dB):     " << snr << " dB" << std::endl;
    
    return 0;
}
