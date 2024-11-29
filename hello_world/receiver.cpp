/* loop_dma.c
 * Demonstrates the use of descriptor base DMA on the FM4. Simply receives and transmits audio samples block wise.
 * Compile together with DSTC_Handler.c
 *
 *  * Recommended slider app parameter settings
 * Index	Name		Minimum		Initial		Maximum		Step Size		Digits
 * 0
 * 1
 * 2
 * 3
 * 4		Line in		0			23			31			1				1.0
 * 5		HP Out		48			48			127			1				1.0
 *
 *
 *
 *
 */

#include <pdl_header.h>
#include "global.h"
//#include "cfar.h"
//#include "kiss_fft.h"
//#include "kiss_fftr.h"
//#include "c_speech_features.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// user definitions
#define SAMPLERATE 96000	// CODEC sampling frequency in Hz, 8000, 32000, 48000 or 96000
//#define BAUDRATE 115200
#define ringBufferSize (8*DMA_BUFFER_SIZE)	// ring buffer size

// user global variables

//float *ringBuffer;	// ring buffer for audio samples
//uint32_t ringBufferIndex = 0;	// index for ring buffer
//kiss_fftr_cfg fft_buffer;
//kiss_fft_scalar *fft_input;
//kiss_fft_cpx *fft_output;


// prototypes

void process_buffer(void);
//void hamming_window(float *buffer, int size);
//void hann_window(float *buffer, int size);

// main function

int main (void)
{
	// Initialize platform resources: I2S, I2C and UART interfaces, CODEC, GPIO for LED, user button and testpin
	// Baudrate is defined in 'platform.h'

	
	//fft_input = (kiss_fft_scalar*)malloc(ringBufferSize * sizeof(kiss_fft_scalar));
	//fft_output = (kiss_fft_cpx*)malloc(ringBufferSize * sizeof(kiss_fft_cpx));
	//ringBuffer = (float*)malloc(ringBufferSize * sizeof(float));


	//fft_buffer = kiss_fftr_alloc(ringBufferSize, 0, NULL, NULL);


	platform_init(BAUDRATE, SAMPLERATE, line_in, dma, DSTC_I2S_HANDLER_CH0, DSTC_I2S_HANDLER_CH1);

	// writeUart0("Hello FM4 World!\r\n");

	while(1)
	{
		while (!(rx_buffer_full && tx_buffer_empty))
		{
			// optional: Update slider parameters, needed in case the slider GUI is used

			//update_slider_parameters(&FM4_GUI);

			// optional: update line in and head phone level through slider app
			// uses slider 4 and 5
			//setLIandHPlevels(&FM4_GUI);

			// toggle activity indicator
			gpio_set(LED_B, HIGH);		// LED_B off
		}

		process_buffer();	// Algorithm
	}
}

void process_buffer(void)
{

	//union audio puffer[DMA_BUFFER_SIZE];
	uint32_t *txbuf, *rxbuf;
	int16_t leftChannel, rightChannel;
	union audio currentSample;


//	gpio_set(TEST_PIN, HIGH);		// toggle TestPin P10 for computing time measurement

	// set txbuf and rxbuf pointer to the active DMA buffer
	// do not modify
	if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping;
	else txbuf = dma_tx_buffer_pong;

	if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping;
	else rxbuf = dma_rx_buffer_pong;

	// enter your data processing algorithms here
	// in this example we just copy the content of the receive buffer into transmit buffer

/*
	for (int i = 0; i < ringBufferSize-DMA_BUFFER_SIZE; i++)
	{
		fft_input[i] = fft_input[i+DMA_BUFFER_SIZE];
	}
*/
	for(int i=0; i < DMA_BUFFER_SIZE; i++)
	{

		// Golden Wire

		*txbuf++ = *rxbuf;		// copy audio sample from receive buffer to transmit buffer

		currentSample.audioSample = *rxbuf++;			// fetch audio sample from receive buffer

		// Ring Buffer which appends the audio samples to the end of the buffer
		
		

		//fft_input[(ringBufferSize-DMA_BUFFER_SIZE)+i] = (kiss_fft_scalar)(currentSample.audio_ch[0])/32768.0f;


		gpio_set(LED_B, LOW);			// LED_B on

		//ringBufferIndex = (ringBufferIndex + 1) % ringBufferSize;


	}


	
	// FFT
	/*
	if (ringBufferIndex == 0)
	{
		
		//hamming_window(fft_input, ringBufferSize);

		hann_window(fft_input, ringBufferSize);


		kiss_fftr(fft_buffer, fft_input, fft_output);

		//free(amplitude);
		float *amplitude = (float*)malloc(ringBufferSize * sizeof(float));

		for (int k = 0; k < ringBufferSize; k++)
		{
			amplitude[k] = sqrt(fft_output[k].r * fft_output[k].r + fft_output[k].i * fft_output[k].i);
		}
	

	// MFCC

	float winLen = 0.025;
	float winStep = 0.01;
	int numCep = 5;
	int nFilters = 10;
	int nfft = 512;
	int lowfreq = 0;
	int highfreq = 4000;
	float preEmph = 0.97;
	int ceplifter = 22;

	float *mfcc_result = (float*)malloc(numCep * sizeof(float));

	short *mfccinput = (short*)malloc(ringBufferSize * sizeof(short));
	for (int i=0; i < ringBufferSize; i++)
	{
		mfccinput[i] = (short)(amplitude[i] * 32768.0f);
	}

	free(amplitude);

	int numMFCCframes = csf_mfcc(mfccinput, ringBufferSize, SAMPLERATE, winLen, winStep, numCep, nFilters, ringBufferSize, lowfreq, highfreq, preEmph, ceplifter, 0, NULL, &mfcc_result);
	
	}
    */
	// Flags for DMA handling, do not modify
	tx_buffer_empty = 0;
	rx_buffer_full = 0;

//	gpio_set(TEST_PIN, LOW);		// toggle TestPin P10 for computing time measurement
}
