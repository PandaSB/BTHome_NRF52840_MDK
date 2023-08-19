/** @file
 *  @brief Bmp180 header
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BMP180_H_
#define _BMP180_H_

#include <stdint.h>
#define BMP180_R_ADDRESS (0x77)

enum _bmp180_oversampling_settings { ultra_low_power, standart, high_resolution, ultra_high_resolution };

/**
 * @struct bmp180_t bmp180.h bmp180.h
 * @brief Holds sensor data, sensor settings and calibration values.
 * */
typedef struct bmp180_t {

    // Sensor data
    float temperature;
    int32_t pressure;
    float altitude;
    int32_t sea_pressure;

    // Settings
    enum _bmp180_oversampling_settings oversampling_setting;
    uint8_t oss;

    // Calibration data
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t B1;
    int16_t B2;
    int32_t B3;
    uint32_t B4;
    int32_t B5;
    int32_t B6;
    uint32_t B7;
    int16_t MB;
    int16_t MC;
    int16_t MD;
} bmp180_t;

int bmp180_begin();
int32_t bmp180_readPressure();      // returns Pressure * 100
int32_t bmp180_readTemperature();   // returns Temperature * 100

#endif