/** @file
 *  @brief Button Service header
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ST_BLE_BUTTON_H_
#define ST_BLE_BUTTON_H_

#include <drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

int button_init(gpio_callback_handler_t handler);

#ifdef __cplusplus
}
#endif

#endif