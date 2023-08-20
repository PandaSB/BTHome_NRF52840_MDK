/** @file
 *  @brief LED Service code
 */

/*
 * Copyright (c) 2023 BARTHELEMY Stéphane
 * base on Marcio Montenegro <mtuxpe@gmail.com> code
 * samples/bluetooth/st_ble_sensor/src/led_svc.c
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <zephyr/types.h>

#include <drivers/gpio.h>
#include <logging/log.h>

#include <led.h>

#define NB_LED 3

LOG_MODULE_REGISTER(led);
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
//static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

static struct gpio_dt_spec led[NB_LED];
static bool led_state[NB_LED]; /* Tracking state here supports GPIO expander-based LEDs. */
static bool led_ok;

/**
 * @brief change led value
 *
 * @param nb index of led
 */
void led_update(int nb) {
    if (!led_ok) {
        return;
    }

    led_state[nb] = !led_state[nb];
    LOG_INF("Turn %s LED", led_state[nb] ? "on" : "off");
    gpio_pin_set(led[nb].port, led[nb].pin, led_state[nb]);
}

/**
 * @brief Set a specific value to led
 *
 * @param nb index led
 * @param value value on/off
 */
void led_set(int nb, bool value) {
    if (!led_ok) {
        return;
    }

    led_state[nb] = value;
    LOG_INF("Turn %s LED", led_state[nb] ? "on" : "off");
    gpio_pin_set(led[nb].port, led[nb].pin, led_state[nb]);
}

/**
 * @brief Init led gpios
 *
 * @return int error code
 */
int led_init(void) {
    int ret;
    memcpy(&led[0], &led0, sizeof(struct gpio_dt_spec));
    memcpy(&led[1], &led1, sizeof(struct gpio_dt_spec));
    memcpy(&led[2], &led2, sizeof(struct gpio_dt_spec));
    // memcpy(&led[3], &led3, sizeof(struct gpio_dt_spec));

    for (int i = 0; i < NB_LED; i++) {
        led_ok = device_is_ready(led[i].port);
        if (!led_ok) {
            LOG_ERR("Error: LED on GPIO %s pin %d is not ready", led[i].port->name, led[i].pin);
            return -ENODEV;
        }

        ret = gpio_pin_configure_dt(&led[i], GPIO_OUTPUT_INACTIVE);
        if (ret < 0) {
            LOG_ERR("Error %d: failed to configure GPIO %s pin %d", ret, led[i].port->name, led[i].pin);
        }
    }

    return ret;
}