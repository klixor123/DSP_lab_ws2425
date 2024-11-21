#include "global.h"

#define BLOCK_SIZE            32
#define NUM_TAPS_ARRAY_SIZE              32
#define NUM_TAPS 5

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLERATE 96000	// CODEC sampling frequency in Hz, 8000, 32000, 48000 or 96000
#define TRANSPORT_FREQ 6000
#define SAMPLEDIVIDER (SAMPLERATE/TRANSPORT_FREQ)




void process_buffer(void)
{

	uint32_t *txbuf, *rxbuf;
	// int16_t leftChannel, rightChannel;
	union audio audioIO;

//	gpio_set(TEST_PIN, HIGH);		// toggle TestPin P10 for computing time measurement

	// set txbuf and rxbuf pointer to the active DMA buffer
	// do not modify
	if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping;
	else txbuf = dma_tx_buffer_pong;

	if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping;
	else rxbuf = dma_rx_buffer_pong;

	// enter your data processing algorithms here
	// in this example we just copy the content of the receive buffer into transmit buffer
    
	for(int i=0; i < DMA_BUFFER_SIZE ; i++)
	{
        
		audioIO.audioSample = *rxbuf++;			// fetch audio sample from receive buffer

	}
	
    for(int i=0; i < DMA_BUFFER_SIZE ; i++)
	{
		

		*txbuf++ = audioIO.audioSample;
	}
	// Flags for DMA handling, do not modify
	tx_buffer_empty = 0;
	rx_buffer_full = 0;

	gpio_set(LED_B, LOW);		// toggle TestPin P10 for computing time measurement
}



int main()
{
    // GPIO port configuration for 3 color LED, user button and test pin
    platform_init(BAUDRATE, SAMPLERATE, line_in, dma, DSTC_I2S_HANDLER_CH0, DSTC_I2S_HANDLER_CH1);

	//GpioInit();

    IF_DEBUG(Uart0Init(115200));

    IF_DEBUG(debug_printf("Hello World!\n"));

    //test_dsp_lib();

    //int i = 0;
    while(true)
    {
        while (!(rx_buffer_full && tx_buffer_empty))
		{
            
        }

		process_buffer();	// Algorithm
        
    }

    return 0;
}
