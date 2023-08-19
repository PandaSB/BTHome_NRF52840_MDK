/** @file
 *  @brief LED Service header
 */

/*
 * Copyright (c) 2023 BARTHELEMY Stéphane
 * base on Marcio Montenegro <mtuxpe@gmail.com> code
 * samples/bluetooth/st_ble_sensor/src/led_svc.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ST_LED_H_
#define _ST_LED_H_

#include <stdbool.h>

void led_update(int nb);
void led_set(int nb, bool value);
int led_init(void);

#endif