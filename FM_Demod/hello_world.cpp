#include "global.h"



// I/O buffer
ping_pong_buffer_instance* input_buffer;
ping_pong_buffer_instance* output_buffer;




void
DSTC_handler_RX()
{
    int32_t* ptr = ping_pong_buffer_get_write_ptr(input_buffer);
    if(ptr == NULL)
    {
        // error: can't write
        fatal_error();
    }
	stcDES[1].DES3 = (uint32_t)ptr;

	Dstc_SetDqmskclrBit(DSTC_IRQ_NUMBER_I2S0_RX);			// clear mask bit for channel 218 (I2S reception)
}


void
DSTC_handler_TX()
{
    int32_t* ptr = ping_pong_buffer_get_read_ptr(output_buffer);
    if(ptr == NULL)
    {
        // error: can't read
        fatal_error();
    }
	stcDES[0].DES2 = (uint32_t)ptr;

	Dstc_SetDqmskclrBit(DSTC_IRQ_NUMBER_I2S0_TX);			// clear mask bit for channel 219 (I2S reception)
}


int 
main()
{
    // GPIO port configuration for 3 color LED, user button and test pin
	GpioInit();

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

    CodecInit(hz48000, line_in);
    I2S0Init(hz48000, dma, nullptr);

    int32_t* rx = ping_pong_buffer_get_write_ptr(input_buffer);
    int32_t* tx = ping_pong_buffer_get_read_ptr(output_buffer);
    init_DSTC(DSTC_handler_RX, rx, DSTC_handler_TX, tx);

    // start I2S
    // I2SEN = 1, clock to I2S macro disabled
	I2s_StartClk (&I2S0);						
	I2s_EnableTx (&I2S0);
	I2s_EnableRx (&I2S0);
	I2s_Start(&I2S0);


    // main loop
    while(true)
    {
        if(ping_pong_buffer_can_read(input_buffer) && ping_pong_buffer_can_write(output_buffer))
        {
            gpio_set(TEST_PIN, HIGH);

            int32_t* input = ping_pong_buffer_get_read_ptr(input_buffer);
            int32_t* output = ping_pong_buffer_get_write_ptr(output_buffer);

            for(uint32_t i = BLOCK_SIZE; i > 0; i--)
            {
                *output++ = *input++;
            }
        
            gpio_set(TEST_PIN, LOW);
        }
    }

    // should not reach here
    fatal_error();

    return 0;
}

