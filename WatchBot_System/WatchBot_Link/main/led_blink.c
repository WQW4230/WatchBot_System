#include "led_blink.h"
#include "driver/gpio.h"

#define LED_BLINK_PIN 2

void Led_Blink_Init(void)
{
    gpio_config_t gpio_configStruct;
    gpio_configStruct.mode = GPIO_MODE_OUTPUT;
    gpio_configStruct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_configStruct.pull_up_en = GPIO_PULLDOWN_DISABLE;
    gpio_configStruct.pin_bit_mask = 1ULL << LED_BLINK_PIN;
    gpio_configStruct.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&gpio_configStruct);
}

void led_Blink_on(void)
{
    gpio_set_level(LED_BLINK_PIN, 1);
}

void led_Blink_off(void)
{
    gpio_set_level(LED_BLINK_PIN, 0);
}