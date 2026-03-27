open_project hls_fft_project
set_top fft_top
add_files fft_hls.cpp
add_files fft_hls_core.cpp
add_files -tb fft_hls_tb.cpp

open_solution "solution2_pipeline"
set_part {xc7z020clg400-1}
create_clock -period 10 -name default

puts "----------------------------------------"
puts "Running C Simulation..."
puts "----------------------------------------"
csim_design

puts "----------------------------------------"
puts "Running C Synthesis..."
puts "----------------------------------------"
csynth_design

exit
