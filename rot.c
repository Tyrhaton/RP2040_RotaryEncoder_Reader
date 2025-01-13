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
        print(uart1,"[-] Encoder Mapping list is full!\n");
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

void encoder_isr(uint gpio, uint32_t events)
{

    rotary_encoder_t *data = find_encoder_by_gpio(gpio);
    uint32_t current_us = current_time_rotary();
    uint32_t last_isr_time = data->last_isr_time;
    data->last_isr_time = current_us;

    if (current_us == last_isr_time)
    {
        return;
    }

    uint8_t current_a = gpio_get(data->gpio_channel_a);
    uint8_t current_b = gpio_get(data->gpio_channel_b);

    if ((current_a != data->last_state_a) || (current_b != data->last_state_b))
    {
        data->last_state_a = current_a;
        data->last_state_b = current_b;

        if (current_a == current_b)
        {
            data->dir = 1 * data->factor; // CW
        }
        else
        {
            data->dir = -1 * data->factor; // CCW
        }

        data->current_value += data->dir;

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
    }
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