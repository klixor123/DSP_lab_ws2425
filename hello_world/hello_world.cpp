#include "global.h"


int main()
{
    // GPIO port configuration for 3 color LED, user button and test pin
	GpioInit();

    IF_DEBUG(Uart0Init(115200));

    IF_DEBUG(debug_printf("Hello World!\n"));

    int i = 0;
    while(true)
    {
        gpio_set(LED_B, LOW);			// LED_B on
        delay_ms(500);
        gpio_set(LED_B, HIGH);			// LED_B off
        delay_ms(500);

        IF_DEBUG(debug_printf("i = %d\n", i));
        i++;
    }

    return 0;
}

