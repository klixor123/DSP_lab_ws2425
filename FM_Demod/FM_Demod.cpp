#include "global.h"
#include <complex.h>
#include <arm_math.h>




// I/O buffer

ping_pong_buffer_instance* input_buffer;
ping_pong_buffer_instance* output_buffer;

//arm_fir_instance_f32 S=

typedef struct{
    double real;
    double imag;
} Complex;


//float32_t hop_size[BLOCK_SIZE/4];

// user global variables
volatile uint8_t rx_proc_buffer = PING;
volatile uint8_t tx_proc_buffer = PING;
volatile boolean_t tx_buffer_empty = FALSE;
volatile boolean_t rx_buffer_full = FALSE;

/* DSTC_HANDLER_CH0: Interrupt handler for DSTC channel 0
 * Channel 0 is used for I2S transmit, toggles the buffer address
 * in channel 0 DES2 between tx ping and pong buffer and restarts
 * the transmission by clearing the DQmask bit for DSTC channel 219
 */
void DSTC_I2S_HANDLER_CH0 (void)
{
	// process tx buffer addresses
	if(tx_proc_buffer == (PONG))
	{
		stcDES[0].DES2 = (uint32_t)&(dma_tx_buffer_pong);	// source address: tx pong buffer
		tx_proc_buffer = PING;								// next buffer is ping
	}
	else
	{
		stcDES[0].DES2 = (uint32_t)&(dma_tx_buffer_ping);	// source address: tx ping buffer
		tx_proc_buffer = PONG;								// next buffer is pong
	}
	tx_buffer_empty = TRUE;                                 // signal to main() that tx buffer empty

	Dstc_SetDqmskclrBit(DSTC_IRQ_NUMBER_I2S0_TX);			// clear mask bit for channel 219 (I2S transmission)
}


/* DSTC_HANDLER_CH1: Interrupt handler for DSTC channel 1
 * Channel 1 is used for I2S receive, toggles the buffer address
 * in channel 1 DES2 between rx ping and pong buffer and restarts
 * the transmission by clearing the DQmask bit for DSTC channel 218
 */
void DSTC_I2S_HANDLER_CH1 (void)
{
	// process rx buffer addresses
   	if(rx_proc_buffer == PONG)
   	{
   		stcDES[1].DES3 = (uint32_t)&(dma_rx_buffer_pong);	// receive address: rx pong buffer
		rx_proc_buffer = PING;								// next buffer is ping
	}
	else
	{
		stcDES[1].DES3 = (uint32_t)&(dma_rx_buffer_ping);	// receive address: rx ping buffer
		rx_proc_buffer = PONG;								// next buffer is ping
	}
	rx_buffer_full = TRUE;									// signal to main() that rx buffer is full

	Dstc_SetDqmskclrBit(DSTC_IRQ_NUMBER_I2S0_RX);			// clear mask bit for channel 218 (I2S reception)
}



/* process_buffer
 * Process the data received from DSTC interrupt handler (DSTC_Handler.c)
 * Caveat: Data processing must be complete before the next DAM buffer is available
 */
void process(void)
{
	int i;
	uint32_t *txbuf, *rxbuf, maxIndex;
	int16_t leftChannel, rightChannel;
	float32_t max;
	float32_t leftChannel_f[BLOCK_SIZE], rightChannel_f[BLOCK_SIZE], leftDemod_f[BLOCK_SIZE];
	//float32_t demod_input[HOP_BUFFER_SIZE], demod_output[HOP_BUFFER_SIZE];
	union audio audioIO;

	// set txbuf and rxbuf pointer to the active DMA buffer
	// do not modify
	if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping;
	else txbuf = dma_tx_buffer_pong;

	if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping;
	else rxbuf = dma_rx_buffer_pong;

	// enter your data processing algorithms here
	// in this example we just copy the content of the receive buffer into transmit buffer

	for(i=0; i < BLOCK_SIZE ; i++)
	{
//		*txbuf++ = *rxbuf++;

		// each sample comprises 16 bit left and right channel data
		audioIO.audioSample = *rxbuf++;			// fetch audio sample from receive buffer
        
        // split channels
		leftChannel = audioIO.audio_ch[LEFT]; 	// extract left channel data
		rightChannel = audioIO.audio_ch[RIGHT];	// extract right channel data

		arm_q15_to_float(&leftChannel, &(leftChannel_f[i]), 1);
		arm_q15_to_float(&rightChannel, &(rightChannel_f[i]), 1);
	}
/**/
	
	arm_max_f32(leftChannel_f, BLOCK_SIZE, &max, &maxIndex);
	//process_demodulation(demod_input, demod_output); 
	if (max > 0.01) {
		float32_t carrier_Freq = Carrier_Freq(leftChannel_f, BLOCK_SIZE);
		process_demodulation(leftChannel_f, leftDemod_f, carrier_Freq);
	}
	else {
		for (uint16_t i = 0; i < BLOCK_SIZE; i++) {
			leftDemod_f[i] = 0;
		}
	}
	//process_demodulation(rightChannel_f, rightDemod_f);
	


	for(i=0; i < BLOCK_SIZE ; i++){
		arm_float_to_q15(&leftDemod_f[i], &leftChannel,1);
		arm_float_to_q15(&leftDemod_f[i], &rightChannel,1);

		//arm_float_to_q15(&rightChannel_f[i], &rightChannel,1);
		//arm_float_to_q15(&leftChannel_f[i], &leftChannel,1);

        // combine channels
		audioIO.audio_ch[LEFT] =  leftChannel;			// write processed data
		audioIO.audio_ch[RIGHT] = rightChannel;			// write processed data

		*txbuf++ = audioIO.audioSample;
	}


	// Flags for DMA handling, do not modify
	tx_buffer_empty = 0;
	rx_buffer_full = 0;

}


int main (void)
{
	// Initialize platform resources: I2S, I2C and UART interfaces, CODEC, GPIO for LED, user button and testpin
	// Baudrate is defined in 'platform.h'
	platform_init(BAUDRATE, CODEC_SAMPLE_RATE, line_in, dma, DSTC_I2S_HANDLER_CH0, DSTC_I2S_HANDLER_CH1);
	DemodInit();
	gpio_set(TEST_PIN, LOW);		// toggle TestPin P10 for computing time measurement
	gpio_set(LED_G, LOW);

	while(1)
	{
		if((rx_buffer_full && tx_buffer_empty))
		{
			gpio_set(LED_G, HIGH);
			gpio_set(LED_R, LOW);

			gpio_set(TEST_PIN, HIGH);		// toggle TestPin P10 for computing time measurement

    		process();	// Algorithm

			gpio_set(TEST_PIN, LOW);		// toggle TestPin P10 for computing time measurement
			gpio_set(LED_G, LOW);
			gpio_set(LED_R, HIGH);
		}
	}
}


/*
int main() {

    // GPIO port configuration for 3 color LED, user button and test pin
	GpioInit();

    arm_rfft_fast_init_f32(&fftHandler, FFT_LENGHT);

    IF_DEBUG(Uart0Init(115200));

    IF_DEBUG(debug_printf("Hello World!\n"));
    IF_DEBUG(debug_printf("Test: %s, %s\n", __DATE__, __TIME__));

    // LED G on
    gpio_set(LED_G, LOW);


    // I/O buffer
    ping_pong_buffer_instance input_buf;
    ping_pong_buffer_instance output_buf;

    input_buffer = &input_buf;
    output_buffer = &output_buf;

    // init all
    ping_pong_buffer_init(input_buffer, write);
    ping_pong_buffer_init(output_buffer, read);

    int16_t r_channel[BLOCK_SIZE];
    int16_t l_channel[BLOCK_SIZE];            


    CodecInit(hz48000, line_in);
    I2S0Init(hz48000, dma, nullptr);

    int32_t* rx = ping_pong_buffer_get_write_ptr(input_buffer);
    int32_t* tx = ping_pong_buffer_get_read_ptr(output_buffer);
    init_DSTC(DSTC_handler_RX, rx, DSTC_handler_TX, tx);
    
    // start I2S
    //I2SEN = 1, clock to I2S macro disabled
	I2s_StartClk (&I2S0);						
	I2s_EnableTx (&I2S0);
	I2s_EnableRx (&I2S0);
	I2s_Start(&I2S0);



    while(true)
    {
        gpio_set(TEST_PIN, LOW);
        //delay_ms(1000);
        if(ping_pong_buffer_can_read(input_buffer) && ping_pong_buffer_can_write(output_buffer))
        {
            gpio_set(TEST_PIN, HIGH);
            int32_t* input = ping_pong_buffer_get_read_ptr(input_buffer);
            int32_t* output = ping_pong_buffer_get_write_ptr(output_buffer);

            // split channels
            for(uint32_t i = 0; i < BLOCK_SIZE; i++)
            {
                r_channel[i] = (int16_t)*input;
                int32_t l_value = (*input) >> 16;
                l_channel[i] = (int16_t)l_value;
            }

            // combine channels
            for(uint32_t i = 0; i < BLOCK_SIZE; i++)
            {
                uint32_t r_value = r_channel[i];
                uint32_t l_value = l_channel[i];
                output[i] = r_value | (l_value << 16);
            }

            gpio_set(TEST_PIN, LOW);
        }
    }
	return 0;
}

*/