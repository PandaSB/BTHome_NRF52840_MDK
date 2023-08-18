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

#include <led.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include <drivers/gpio.h>

#include <logging/log.h>


LOG_MODULE_REGISTER(led);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static bool led_state; /* Tracking state here supports GPIO expander-based LEDs. */
static bool led_ok;

void led_update(void)
{
	if (!led_ok) {
		return;
	}

	led_state = !led_state;
	LOG_INF("Turn %s LED", led_state ? "on" : "off");
	gpio_pin_set(led.port, led.pin, led_state);
}

int led_init(void)
{
	int ret;

	led_ok = device_is_ready(led.port);
	if (!led_ok) {
		LOG_ERR("Error: LED on GPIO %s pin %d is not ready",
			led.port->name, led.pin);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Error %d: failed to configure GPIO %s pin %d",
			ret, led.port->name, led.pin);
	}

	return ret;
}