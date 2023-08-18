/** @file
 *  @brief Bmp280 header
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BMP280_H_
#define _BMP280_H_

#include <stdint.h>
#define BMP280_R_ADDRESS (0x77)
int bmp280_begin();
int32_t bmp280_readPressure(); // returns Pressure * 100
int32_t bmp280_readTemperature(); // returns Temperature * 100
int bmp280_readRegister(uint8_t RegNum, uint8_t *Value);
int bmp280_writeRegister(uint8_t RegNum, uint8_t Value);
void bmp280_readCalibrationData(void);

#endif 