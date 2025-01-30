#include "global.h"
#include <arm_math.h>
#include "arm_const_structs.h"

#define FILTER_ORDER 6
#define N 64
#define FS 48000

arm_rfft_fast_instance_f32 fftHandler;
arm_cfft_instance_f32 cfftHandler;
arm_fir_instance_f32 firInstance;

float32_t rfftBufout[BLOCK_SIZE];
float32_t ifftBuf[BLOCK_SIZE];
float32_t current_phase[BLOCK_SIZE]; 
float32_t inputComplex[2 * BLOCK_SIZE];
float32_t HilbertInput[2*BLOCK_SIZE] = {0.0f};
float32_t HilbertFiltered[BLOCK_SIZE] = {0.0f};
float32_t inst_freq[BLOCK_SIZE];
float32_t temp[BLOCK_SIZE];
float32_t mag[BLOCK_SIZE];
uint32_t fftindex = 0;
float32_t carrier_Freq = 0.0f;
float32_t prev_phase = 0.0f;
float32_t sum = 0.0f;
float32_t mean = 0.0f;
float32_t max = 0.0f;
static float32_t last_phase = 0.0f;
float coefficients[FILTER_TAPS];
bool fftflag=0;

const int fir_length = 101;
const float32_t firCoeffs[101] = {
  3.307972918e-07,6.547236922e-07,-3.793598115e-21,-3.554487876e-06,-6.536736691e-06,
  6.828819274e-20,1.732316014e-05,2.617248902e-05,-7.570804161e-20,-5.440074165e-05,
  -7.568594447e-05,-2.338972865e-19, 0.000138688687,0.0001834363648,-4.062449166e-19,
  -0.000308671355,-0.0003935915011,2.810693547e-18,0.0006211409927,0.0007698875852,
  -1.338521437e-18,-0.001155016012,-0.001399520785,2.144115013e-18, 0.002015665174,
   0.002397949575,-3.210214338e-18, -0.00334162591, -0.00391730573,4.526850954e-18,
    0.00531884376, 0.006165371742,-6.045874678e-18,-0.008215236478,-0.009452651255,
  7.679311542e-18,  0.01246966515,  0.01431624684,-9.305246705e-18, -0.01894140989,
    -0.0218831636,1.078126448e-17,  0.02973994426,  0.03518769518,-1.196364803e-17,
   -0.05206223577, -0.06643639505,1.272897952e-17,   0.1365766823,   0.2750346661,
     0.3333332837,   0.2750346661,   0.1365766823,1.272897952e-17, -0.06643639505,
   -0.05206223577,-1.196364803e-17,  0.03518769518,  0.02973994426,1.078126448e-17,
    -0.0218831636, -0.01894140989,-9.305246705e-18,  0.01431624684,  0.01246966515,
  7.679311542e-18,-0.009452651255,-0.008215236478,-6.045874678e-18, 0.006165371742,
    0.00531884376,4.526850954e-18, -0.00391730573, -0.00334162591,-3.210214338e-18,
   0.002397949575, 0.002015665174,2.144115013e-18,-0.001399520785,-0.001155016012,
  -1.338521437e-18,0.0007698875852,0.0006211409927,2.810693547e-18,-0.0003935915011,
  -0.000308671355,-4.062449166e-19,0.0001834363648, 0.000138688687,-2.338972865e-19,
  -7.568594447e-05,-5.440074165e-05,-7.570804161e-20,2.617248902e-05,1.732316014e-05,
  6.828819274e-20,-6.536736691e-06,-3.554487876e-06,-3.793598115e-21,6.547236922e-07,
  3.307972918e-07
};

// FIR Bandpassfilter
const float32_t firCoeffsBP[] = {

    -0.000000000000000008,    -0.000000000000000003,    0.000000000131494038,    0.000000000000000003,
    -0.000000000692312345,    -0.000000000000000003,    -0.000000000157786751,    0.000000000000000000,
    0.000000003480799923,    -0.000000000000000002,    0.000000003036598509,    -0.000000000000000005,
    -0.000000008900224907,    0.000000000000000001,    -0.000000016078908590,    0.000000000000000004,
    0.000000009303467384,    0.000000000000000002,    0.000000046264901928,    0.000000000000000001,
    0.000000019531909150,    0.000000000000000000,    -0.000000083456662486,    0.000000000000000003,
    -0.000000127854770578,    0.000000000000000002,    0.000000028993248712,    0.000000000000000008,
    0.000000310051170722,    -0.000000000000000001,    0.000023385670304785,    -0.000000005397312531,
    0.000142222720093568,    -0.000000000000000002,    -0.000452771219911761,    0.000000063947042468,
    -0.001687961717141683,    0.000000083229045828,    -0.000000063516848630,    -0.000000155479815315,
    0.005948396817213300,    -0.000000434396021296,    0.006157483543284588,    -0.000000000000000015,
    -0.009823125666239059,    0.000001109245794609,    -0.024480292166285680,    0.000001042558345850,
    -0.000000005882180343,    -0.000001563085681740,    0.055302157484702880,    -0.000003815574030281,
    0.052350184039383532,    -0.000000000000000026,    -0.086582187405971373,    0.000010827436509875,
    -0.296937354272971077,    0.000020865263811914,    0.600023460343905524,    0.000020865263811890,
    -0.296937354272971521,    0.000010827436509878,    -0.086582187405971067,    -0.000000000000000026,
    0.052350184039383435,    -0.000003815574030286,    0.055302157484702907,    -0.000001563085681737,
    -0.000000005882180402,    0.000001042558345851,    -0.024480292166285708,    0.000001109245794601,
    -0.009823125666239010,    -0.000000000000000008,    0.006157483543284660,    -0.000000434396021301,
    0.005948396817213313,    -0.000000155479815318,    -0.000000063516848649,    0.000000083229045829,
    -0.001687961717141684,    0.000000063947042468,    -0.000452771219911730,    -0.000000000000000004,
    0.000142222720093585,    -0.000000005397312530,    0.000023385670304766,    0.000000000000000000,
    0.000000310051170708,    0.000000000000000003,    0.000000028993248763,    0.000000000000000007,
    -0.000000127854770598,    -0.000000000000000002,    -0.000000083456662488,    0.000000000000000001,
    0.000000019531909143,    0.000000000000000001,    0.000000046264901960,    0.000000000000000000,
    0.000000009303467387,    0.000000000000000000,    -0.000000016078908601,    -0.000000000000000007,
    -0.000000008900224912,    -0.000000000000000003,    0.000000003036598521,    -0.000000000000000001,
    0.000000003480799925,    0.000000000000000000,    -0.000000000157786772,    0.000000000000000003,
    -0.000000000692312343,    0.000000000000000001,    0.000000000131494052,    0.000000000000000000,
    0.000000000000000005
};



void DemodInit()
{
	arm_rfft_fast_init_f32(&fftHandler, FFT_SIZE);
	
	//arm_cfft_init_f32(&arm_cfft_sR_f32_len32, FFT_SIZE); 	
}


float32_t Carrier_Freq(float32_t* input, uint32_t length)
{
	float32_t fft_input_buffer[FFT_SIZE];
	float32_t fft_output_buffer[FFT_SIZE];
	float32_t fft_magnitude[FFT_SIZE / 2];
	float32_t magnitude[BLOCK_SIZE];

	// Copy input to FFT buffer and zero-pad if necessary
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        if (i < length){
            fft_input_buffer[i] = input[i];
        }
        else
            fft_input_buffer[i] = 0.0f;
    }	

	// Perform FFT
    arm_rfft_fast_f32(&fftHandler, fft_input_buffer, fft_output_buffer, 0);

    // Calculate magnitude
    arm_cmplx_mag_f32(fft_output_buffer, fft_magnitude, FFT_SIZE / 2);


	// Finde die maximale Magnitude und den entsprechenden Index
	float32_t max_value;
	uint32_t max_index;
	arm_max_f32(magnitude, BLOCK_SIZE, &max_value, &max_index);

	float32_t dominant_frequency = (float32_t)max_index * SAMPLE_FREQ / BLOCK_SIZE;
    	//debug_printf("Dominant Frequency: %f Hz\n", dominant_frequency);

	/*if ((float32_t)max_index * SAMPLE_FREQ / BLOCK_SIZE >= 1000.0f &&
    (float32_t)max_index * SAMPLE_FREQ / BLOCK_SIZE <= 10000.00f) 
	{
    	float32_t dominant_frequency = (float32_t)max_index * SAMPLE_FREQ / BLOCK_SIZE;
    	debug_printf("Dominant Frequency: %f Hz\n", dominant_frequency);
    	return dominant_frequency;
	} 
	else 
	{
    	//debug_printf("Frequency out of bounds\n");
    	return 0.0f;
	}*/
	return dominant_frequency;
}


void arm_chilbert_f32(const arm_cfft_instance_f32* S, float32_t* p1) 
{
	uint32_t i, L;
	L = (S->fftLen);
	
	arm_cfft_f32(S, p1, 0, 1);								//p1 = [Re{0}, Im{0}, Re{1}, Im{1},...,Re{Nyquist},Im{Nyquist}=0, Negative Freq. ...]

	for(i = 0; i <= (L*2)-1; i += 2){
		if(i == 0 || i == L) {}
		else if(i <= L-2) {
			p1[i] =  2*p1[i];
			p1[i+1] =  2*p1[i+1];
		}
		else {
			p1[i] = 0;
			p1[i+1] = 0;
		}
	}
	
	arm_cfft_f32(S, p1, 1, 1);					

	//for(uint16_t i=0; i<BLOCK_SIZE;i++){
	//	p1[i]=sqrt(p1[i*2]*p1[i*2]+p1[i*2+1]*p1[i*2+1]);		//MAginute berechnen.
	//}	
}

float32_t calculate_instantaneous_frequency(float32_t* phase_current, float32_t* phase_previous, float32_t carrier_Freq) {
	float32_t delta_phi = (*phase_current - *phase_previous);


	// Phasensprünge verhindern. Es könnte, bedingt durch den atan2 zu einer Phasendifferenz von >PI kommen, wenn atan von Pi auf -Pi sprint.
	// Der atan2 liefert ein Ergebnis zwischen -Pi und Pi, jedoch kann es zu einem Sprung vom einen auf den anderen kommen (-2,9 zu 2,9) Technisch 
	// gesehen ist die differenz 6,jedoch ist die Differenz auf dem Einheitskrei eigentlich sehr klein
	if (delta_phi > M_PI) {
		delta_phi -= 2 * M_PI; // Phasen-Sprung nach unten
	}
	else if (delta_phi < -M_PI) {
		delta_phi += 2 * M_PI; // Phasen-Sprung nach oben
	}
	float32_t inst_freq = ((delta_phi*SAMPLE_FREQ)/(2*PI*carrier_Freq)); 
	return inst_freq;
}


#if 1
void process_demodulation(float32_t input[BLOCK_SIZE], float32_t output[BLOCK_SIZE], float32_t carrier)
{ 	
		
	arm_fir_init_f32(&firInstance, 117, firCoeffsBP, temp, BLOCK_SIZE);
	arm_fir_f32(&firInstance, input, input, BLOCK_SIZE);

	for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
		HilbertInput[2 * i] = input[i];     		// Realteil
    	HilbertInput[2 * i + 1] = 0;                // Imaginärteil (0, da es ein reales Signal ist)
		//debug_printf("HilbertInput: %f\n", HilbertInput[2*i]);
		//debug_printf("HilbertInput: %f\n", HilbertInput[2*i+1]);
	}
	

	arm_chilbert_f32(&arm_cfft_sR_f32_len2048, HilbertInput);				//FFT Länge muss halb so groß sein wie der Input
	for (int16_t i = 0; i < BLOCK_SIZE; i++)
    {  
        // Berechne die Phase für die aktuelle Frequenzkomponente
        current_phase[i] = atan2f(HilbertInput[2*i+1], HilbertInput[2*i]); 				// atan2 gibt die Phase in Bogenmaß zurück
		if ( i > 0) {
			prev_phase = current_phase[i-1];
		}
		else {
			prev_phase = last_phase;
		}
		output[i] = (calculate_instantaneous_frequency(&current_phase[i], &prev_phase, carrier));
			
	}
	last_phase = current_phase[BLOCK_SIZE-1];
	//bandpass_filter(output);
	// Tiefpassfilter
	arm_fir_init_f32(&firInstance, fir_length, firCoeffs, temp, BLOCK_SIZE);
	arm_fir_f32(&firInstance, output, output, BLOCK_SIZE);

	sum = 0.0f;
	for (int16_t i = 0; i < BLOCK_SIZE; i++) {
		sum += output[i];
	}
	mean = sum / BLOCK_SIZE ;
	for (int16_t i = 0; i < BLOCK_SIZE; i++) {
		if (output[i] > 0) {
			output[i] = output[i] - mean;
		}
		else {
			output[i] = output[i] + mean;
		}
	}

	// Normierung
	max = 0.0f;
	for (int16_t i = 0; i < BLOCK_SIZE; i++) {
		max = fmaxf(max, fabsf(output[i]));
	}
	for (int16_t i = 0; i < BLOCK_SIZE; i++) {
		output[i] /= max;
	}
	
	//fir_filter(inst_freq, output ,firCoeffs32,BLOCK_SIZE);
}
#endif

/*************** 
 * DC-Komponente?
 * negative input Anteile?
 * return delta_phi ?
 * Normierungen?
 * 
 * 
*/