#pragma once 
void DemodInit();
void process_demodulation(float32_t input[BLOCK_SIZE], float32_t output[BLOCK_SIZE]);
void HilbertFilter(float32_t *filter, uint32_t size);
float32_t calculate_phase(float32_t* real, float32_t* imag);
float32_t calculate_instantaneous_frequency(float32_t* phase_current, float32_t* phase_previous);

//void arm_chilbert_f32(const arm_cfft_instance_f32*, float32_t*);