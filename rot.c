#include "rot.h"

#define MAX_MAPPINGS 20

static gpio_encoder_mapping_t gpio_encoder_map[MAX_MAPPINGS];
static int mapping_count = 0;

uint32_t current_time_rotary()
{
    return (uint32_t)(time_us_32() / 100000);
}

void add_gpio_encoder_mapping(uint gpio_pin, rotary_encoder_t encoder)
{
    if (mapping_count < MAX_MAPPINGS)
    {
        gpio_encoder_map[mapping_count].gpio_pin = gpio_pin;
        gpio_encoder_map[mapping_count].encoder = encoder;
        mapping_count++;
    }
    else
    {
        print(uart1, "[-] Encoder Mapping list is full!\n");
    }
}

rotary_encoder_t *find_encoder_by_gpio(uint gpio_pin)
{
    for (int i = 0; i < mapping_count; i++)
    {
        if (gpio_encoder_map[i].gpio_pin == gpio_pin)
        {
            return &gpio_encoder_map[i].encoder;
        }
    }
    return &gpio_encoder_map[0].encoder;
}

int a = 0;
int b = 0;
int c = 0;
void encoder_isr(uint gpio, uint32_t events)
{

    // print(uart1, "[-] ISR\n\r");

    uint32_t save = save_and_disable_interrupts();
    rotary_encoder_t *data = find_encoder_by_gpio(gpio);
    uint32_t current_us = current_time_rotary();
    uint32_t last_isr_time = data->last_isr_time;
    data->last_isr_time = current_us;

    if (current_us == last_isr_time)
    {
        restore_interrupts_from_disabled(save);

        return;
    }

    uint8_t current_a = gpio_get(data->gpio_channel_a);
    uint8_t current_b = gpio_get(data->gpio_channel_b);
    // print(uart1, "[+] a: %d b:%d\n\r", current_a, current_b);

    // sleep_us(10);

    // uint8_t current_a2 = gpio_get(data->gpio_channel_a);
    // uint8_t current_b2 = gpio_get(data->gpio_channel_b);

    // // current_a = current_a2;
    // // current_b = current_b2;

    // print(uart1, "[+] a: %d:%d:%d\n\r", data->last_state_a, current_a, current_a2);
    // print(uart1, "[+] b: %d:%d:%d\n\r", data->last_state_b, current_b, current_b2);

    // // print(uart1, "[+] a: %d:%d\n\r", current_a, current_a2);
    // // print(uart1, "[+] b: %d:%d\n\r", current_b, current_b2);

    // if (current_a != current_a2 || current_b != current_b2)
    // {
    //     print(uart1, "[-] Bad\n\r");
    // }
    int a_total = 0;
    int b_total = 0;

    int amt = 10;
    for (int i = 0; i < amt; i++)
    {
        sleep_us(1);

        a_total += gpio_get(data->gpio_channel_a);
        b_total += gpio_get(data->gpio_channel_b);
    }
    if (a_total > 1){
        current_a = 1;
    }
    if (b_total > 1){
        current_b = 1;
    }
    // print(uart1, "[+] a: %d b:%d\n\r", a_total, b_total);

    if (current_a == 1 && a_total == 0)
    {
        restore_interrupts_from_disabled(save);

        return;
    }
    if (current_a == 0 && a_total > 0)
    {
        restore_interrupts_from_disabled(save);

        return;
    }
    ////////////////

    // if (current_a == 0 && a_total == 0 && current_b == 1 && b_total == 10)
    // {
    //     restore_interrupts_from_disabled(save);

    //     return;
    // }

    // if (current_a == 1 && a_total == 10 && current_b == 1 && b_total == 10)
    // {
    //     restore_interrupts_from_disabled(save);

    //     return;
    // }

    // {
    //     sleep_us(10);

    //     uint8_t current_a3 = gpio_get(data->gpio_channel_a);
    //     uint8_t current_b3 = gpio_get(data->gpio_channel_b);

    //     // print(uart1, "[+] a: %d:%d:%d:%d\n\r", data->last_state_a, current_a, current_a2, current_a3);
    //     // print(uart1, "[+] b: %d:%d:%d:%d\n\r", data->last_state_b, current_b, current_b2, current_b3);

    //     print(uart1, "[+] a: %d:%d:%d\n\r", current_a, current_a2, current_a3);
    //     print(uart1, "[+] b: %d:%d:%d\n\r", current_b, current_b2, current_b3);

    //     current_a = current_a3;
    //     current_b = current_b3;
    // }

    // if ((current_a != data->last_state_a) || (current_b != data->last_state_b))
    // {

    // if (current_a != data->last_state_a && current_a == 1)
    // {
    //     if (current_b != current_a)
    //     {
    //         data->dir = 1;
    //         data->current_value += 1 * data->factor; // CW
    //     }
    //     else
    //     {
    //         data->dir = -1;

    //         data->current_value += -1 * data->factor; // CCW
    //     }
    // }
    data->last_state_a = current_a;
    data->last_state_b = current_b;

    if (current_a == current_b)
    {
        data->dir = 1;
        data->current_value += 1 * data->factor; // CW
    }
    else
    {
        data->dir = -1;

        data->current_value += -1 * data->factor; // CCW
    }

    // data->current_value += data->dir;

    switch (data->type)
    {
    case 1:
        if (data->current_value < data->min_value)
        {
            data->current_value = data->max_value;
        }
        if (data->current_value > data->max_value)
        {
            data->current_value = data->min_value;
        }
        break;
    default:

        if (data->current_value < data->min_value)
        {
            data->current_value = data->min_value;
        }
        if (data->current_value > data->max_value)
        {
            data->current_value = data->max_value;
        }
        break;
    }
    print(uart1, "[-] => Encdr: %d\n\r", data->current_value);

    if (a == data->current_value)
    {
        print(uart1, "[+] Value Rot: %d\n\r", data->current_value);
        print(uart1, "[+] a: %d b:%d\n\r", current_a, current_b);

        print(uart1, "[+] a: %d b:%d\n\r", a_total, b_total);
    }
    a = b;
    b = data->current_value;

    // }
    // else
    // {
    //     print(uart1, "[-] Bad\n\r");
    //     print(uart1, "[-] dir: %d\n\r", data->dir);
    // }

    restore_interrupts_from_disabled(save);
}

rotary_encoder_t create_rotary(int gpio_channel_a, int gpio_channel_b, int min_value, int max_value, int factor, int current_value, int type)
{
    rotary_encoder_t encoder;
    encoder.gpio_channel_a = gpio_channel_a;
    encoder.gpio_channel_b = gpio_channel_b;
    encoder.min_value = min_value;
    encoder.max_value = max_value;
    encoder.factor = factor;
    encoder.current_value = current_value;
    encoder.last_isr_time = 0;
    encoder.debounce_edge = 5;
    encoder.last_state_a = 0;
    encoder.last_state_b = 0;
    encoder.type = type;
    encoder.dir = 1;

    add_gpio_encoder_mapping(gpio_channel_a, encoder);

    gpio_init(gpio_channel_a);
    gpio_set_dir(gpio_channel_a, GPIO_IN);
    gpio_pull_up(gpio_channel_a);
    add_gpio_encoder_mapping(gpio_channel_a, encoder);
    gpio_set_irq_enabled_with_callback(gpio_channel_a, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_isr);

    gpio_init(gpio_channel_b);
    gpio_set_dir(gpio_channel_b, GPIO_IN);
    gpio_pull_up(gpio_channel_b);

    irq_set_enabled(IO_IRQ_BANK0, true);

    return encoder;
}

int16_t get_rotary_value(int gpio_channel_a)
{
    rotary_encoder_t *data = find_encoder_by_gpio(gpio_channel_a);
    return data->current_value;
}