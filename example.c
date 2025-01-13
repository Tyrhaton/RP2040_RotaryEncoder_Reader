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

    int16_t last_value_rot = 0;

    while (1)
    {
        int16_t prev_value_rot = last_value_rot;
        last_value_rot = get_rotary_value(encoder.gpio_channel_a);

        if (prev_value_rot == last_value_rot)
        {
            continue;
        }

        print(uart1, "[+] Current Rotary Value on GPIO %d/%d: %d\r\n", encoder.gpio_channel_a, encoder.gpio_channel_b, last_value_rot);
        sleep_ms(500);
    }

    return 0;
}
