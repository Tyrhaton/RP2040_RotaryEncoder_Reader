#include "pico/stdlib.h"
#include "rot.h"
#include "uart.h"

#define UART_ID uart1
#define Baudrate 115200
#define TX 4
#define RX 5

int main()
{
    stdio_init_all();
    init_uart(UART_ID, Baudrate, TX, RX);

    rotary_encoder_t encoder = create_rotary(10, 11, 0, 5, 1, 0, 1);

    extern volatile int Last_Rotary_Value;
	while (1)
	{
        print(uart1, "%d\n\r", Last_Rotary_Value);
		sleep_ms(50);
	}

    return 0;
}
