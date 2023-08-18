/** @file
 *  @brief LED Service 
 */

/*
 * Copyright (c) 2023 BARTHELEMY Stéphane 
 * base on Marcio Montenegro <mtuxpe@gmail.com> code
 * samples/bluetooth/st_ble_sensor/src/led_svc.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ST_BLE_LED_H_
#define ST_BLE_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

void led_update(void);
int led_init(void);

#ifdef __cplusplus
}
#endif

#endif