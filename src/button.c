/** @file
 *  @brief Button Service sample
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "button.h"

#include <kernel.h>
#include <logging/log.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>

LOG_MODULE_REGISTER(button_svc);

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback gpio_cb;

/**
 * @brief init button
 *
 * @param handler pointer on button structure
 * @return int error code
 */
int button_init(gpio_callback_handler_t handler) {
    int ret;

    if (!device_is_ready(button.port)) {
        LOG_ERR("Error: button GPIO device %s is not ready", button.port->name);
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Error %d: can't configure button on GPIO %s pin %d", ret, button.port->name, button.pin);
        return ret;
    }

    gpio_init_callback(&gpio_cb, handler, BIT(button.pin));
    gpio_add_callback(button.port, &gpio_cb);
    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Error %d: can't configure button interrupt on "
                "GPIO %s pin %d",
                ret, button.port->name, button.pin);
        return ret;
    }
    return 0;
}