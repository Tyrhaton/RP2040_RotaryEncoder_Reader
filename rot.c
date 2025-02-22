#include "rot.h"

#define MAX_MAPPINGS 20

static gpio_encoder_mapping_t gpio_encoder_map[MAX_MAPPINGS];
static int mapping_count = 0;

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

int ccw_cw = 1;
volatile int Last_Rotary_Value = 0;
void encoder_callback(uint gpio, uint32_t events)
{
    uint32_t save = save_and_disable_interrupts();

    uint32_t gpio_state = (gpio_get_all() >> 10) & 0b0111; // get all GPIO then mask out bits 10, 11, 12
    uint8_t enc_value = gpio_state & 0x03;                 // use only bits 0 and 1 for A and B
    rotary_encoder_t *data = find_encoder_by_gpio(gpio);

    // Static flags for detecting rising edges
    static bool ccw_fall = false;
    static bool cw_fall = false;

    if (gpio == data->gpio_channel_a)
    {
        if (enc_value == 0b10) // Channel A rising edge indicates potential CW movement.
        {
            cw_fall = true;
            ccw_fall = false; // Clear CCW flag to avoid conflicting triggers.
        }
        else if (enc_value == 0b00 && ccw_fall) // If CCW flag is set and we see 00, complete a CCW cycle.
        {
            cw_fall = false;
            ccw_fall = false;

            if (ccw_cw == 1)
            {
                print(uart1, "switch %d\n\r", Last_Rotary_Value);
            }
            Last_Rotary_Value -= 1 * data->factor;
            if (Last_Rotary_Value < 0)
            {
                Last_Rotary_Value = 5;
            }
            print(uart1, "CCW %d:\n\r", Last_Rotary_Value);
            ccw_cw = 0;
        }
    }
    else if (gpio == data->gpio_channel_b)
    {
        if (enc_value == 0b01) // Channel B rising edge indicates potential CCW movement.
        {
            ccw_fall = true;
            cw_fall = false; // Clear CW flag to avoid conflicting triggers.
        }
        else if (enc_value == 0b00 && cw_fall) // If CW flag is set and we see 00, complete a CW cycle.
        {
            cw_fall = false;
            ccw_fall = false;
            if (ccw_cw == 0)
            {
                print(uart1, "switch %d\n\r", Last_Rotary_Value);
            }
            Last_Rotary_Value += 1 * data->factor;
            if (Last_Rotary_Value > 5)
            {
                Last_Rotary_Value = 0;
            }
            print(uart1, "CW  %d:\n\r", Last_Rotary_Value);
            // print(uart1, "%d\n\r", data->dir);

            // data->dir = 1;
            ccw_cw = 1;
        }
    }
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
    encoder.ccw_cw = 0;

    add_gpio_encoder_mapping(gpio_channel_a, encoder);
    add_gpio_encoder_mapping(gpio_channel_b, encoder);

    gpio_init(gpio_channel_a);
    gpio_set_dir(gpio_channel_a, GPIO_IN);
    gpio_disable_pulls(gpio_channel_a);

    gpio_init(gpio_channel_b);
    gpio_set_dir(gpio_channel_b, GPIO_IN);
    gpio_disable_pulls(gpio_channel_b);

    gpio_set_irq_enabled_with_callback(gpio_channel_a, GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    gpio_set_irq_enabled(gpio_channel_b, GPIO_IRQ_EDGE_FALL, true);

    return encoder;
}

int16_t get_rotary_value(int gpio_channel_a)
{
    rotary_encoder_t *data = find_encoder_by_gpio(gpio_channel_a);
    return data->current_value;
}