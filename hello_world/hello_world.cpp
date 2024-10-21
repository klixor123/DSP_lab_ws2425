#include <stdio.h>

extern "C"
{
#include <platform.h>
#include <utils.h>
}



int main()
{
   	// Initialize platform resources: I2S, I2C and UART interfaces, CODEC, GPIO for LED, user button and test pin
	//platform_init(BAUDRATE, SAMPLERATE, line_in, intr, I2S_HANDLER, NULL); // second pointer to interrupt handler only for DSTC needed
    
    // GPIO port configuration for 3 color LED, user button and test pin
	GpioInit();


    printf("Hello World!\n");

    while(true)
    {
        gpio_set(LED_B, LOW);			// LED_B on
        delay_ms(300);
        gpio_set(LED_B, HIGH);			// LED_B off
        delay_ms(300);
    }

    return 0;
}

