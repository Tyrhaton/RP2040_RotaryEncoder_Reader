#ifndef ROTARYENCODER_H
#define ROTARYENCODER_H

#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "uart.h"
#include "led.h"

typedef struct ROTARY_ENCODER_DATA
{
    uint8_t gpio_channel_a, gpio_channel_b;
    int16_t min_value,current_value, max_value;
    int8_t factor, dir;
    uint8_t last_state_a, last_state_b;
    uint32_t last_isr_time, debounce_edge;
    int8_t type;
} rotary_encoder_t;

typedef struct
{
    int gpio_pin;
    rotary_encoder_t encoder;
} gpio_encoder_mapping_t;

rotary_encoder_t create_rotary(int gpio_channel_a, int gpio_channel_b, int min_value, int max_value, int factor, int current_value, int type);
int16_t get_rotary_value(int gpio_a);

#endif