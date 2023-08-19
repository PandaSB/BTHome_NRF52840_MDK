/** @file
 *  @brief Bmp180 Service code
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bmp180.h"
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <drivers/spi.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/printk.h>

// Registers
#define CALIB     0xAA
#define ID        0xD0
#define SOFT      0xE0
#define CTRL_MEAS 0xF4
#define OUT_MSB   0xF6
#define OUT_LSB   0xF7
#define OUT_XLSB  0xF8

// Private defines
#define convert8bitto16bit(x, y) (((x) << 8) | (y))
#define powerof2(x)              (1 << (x))

static uint8_t id = 0;

static const struct device *i2c;
bmp180_t bmp180;

/**
 * @brief Read register from bmp180
 *
 * @param RegNum index of register
 * @param Value pointer on regiser value
 * @return int error code
 */
int bmp180_readRegister(uint8_t RegNum, uint8_t *Value) {
    // reads a series of bytes, starting from a specific register
    int nack;
    nack = i2c_reg_read_byte(i2c, BMP180_R_ADDRESS, RegNum, Value);
    return nack;
}

int bmp180_writeRegister(uint8_t RegNum, uint8_t Value) {
    // sends a byte to a specific registertion
    uint8_t Buffer[2];
    Buffer[0] = Value;
    int nack;
    nack = i2c_reg_write_byte(i2c, BMP180_R_ADDRESS, RegNum, Value);
    return nack;
}

/**
 * @brief Read calibration data from bmp180
 *
 */
void bmp180_readCalibrationData() {
    uint8_t CalibrationData[26];
    int i;
#define CAL_START 0xAA   // Defining the address where the calibration data should start
    for (i = 0; i < 22; i++) {
        // Read calibration table
        bmp180_readRegister(CAL_START + i, &CalibrationData[i]);
    }

    for (uint8_t i = 0; i < 22; i += 2) {
        uint16_t combined_calibration_data = convert8bitto16bit(CalibrationData[i], CalibrationData[i + 1]);
        if (combined_calibration_data == 0x00 || combined_calibration_data == 0XFF) {
            printk("Error calibration setting \r\n");
            return;
        }
    }

    switch (bmp180.oversampling_setting) {
    case ultra_low_power:
        bmp180.oss = 0;
        break;
    case standart:
        bmp180.oss = 1;
        break;
    case high_resolution:
        bmp180.oss = 2;
        break;
    case ultra_high_resolution:
        bmp180.oss = 3;
        break;
    default:
        bmp180.oversampling_setting = standart;
        bmp180.oss = 1;
        break;
    }

    // Save calibration data
    bmp180.AC1 = convert8bitto16bit(CalibrationData[0], CalibrationData[1]);
    bmp180.AC2 = convert8bitto16bit(CalibrationData[2], CalibrationData[3]);
    bmp180.AC3 = convert8bitto16bit(CalibrationData[4], CalibrationData[5]);
    bmp180.AC4 = convert8bitto16bit(CalibrationData[6], CalibrationData[7]);
    bmp180.AC5 = convert8bitto16bit(CalibrationData[8], CalibrationData[9]);
    bmp180.AC6 = convert8bitto16bit(CalibrationData[10], CalibrationData[11]);
    bmp180.B1 = convert8bitto16bit(CalibrationData[12], CalibrationData[13]);
    bmp180.B2 = convert8bitto16bit(CalibrationData[14], CalibrationData[15]);
    bmp180.B3 = 0;
    bmp180.B4 = 0;
    bmp180.B5 = 0;
    bmp180.B6 = 0;
    bmp180.B7 = 0;
    bmp180.MB = convert8bitto16bit(CalibrationData[16], CalibrationData[17]);
    bmp180.MC = convert8bitto16bit(CalibrationData[18], CalibrationData[19]);
    bmp180.MD = convert8bitto16bit(CalibrationData[20], CalibrationData[21]);
    bmp180.sea_pressure = 101325;
}

/**
 * @brief read up data from bmp180
 *
 * @return int32_t return up value
 */
static int32_t bmp180_read_up() {
    uint8_t write_data = 0x34 + (bmp180.oss << 6), up_data[3];
    bmp180_writeRegister(CTRL_MEAS, write_data);
    uint8_t wait = 0;
    switch (bmp180.oversampling_setting) {
    case ultra_low_power:
        wait = 5;
        break;
    case standart:
        wait = 8;
        break;
    case high_resolution:
        wait = 14;
        break;
    case ultra_high_resolution:
        wait = 26;
        break;
    default:
        wait = 5;
        break;
    }
    k_msleep(wait);
    bmp180_readRegister(OUT_MSB, &up_data[0]);
    bmp180_readRegister(OUT_MSB + 1, &up_data[1]);
    bmp180_readRegister(OUT_MSB + 2, &up_data[2]);

    return ((up_data[0] << 16) + (up_data[1] << 8) + up_data[2]) >> (8 - bmp180.oss);
}

/**
 * @brief read ut data frol bmp180
 *
 * @return int16_t return ut data
 */
static int16_t bmp180_read_ut() {
    uint8_t write_data = 0x2E, ut_data[2];

    bmp180_writeRegister(CTRL_MEAS, write_data);
    k_msleep(5);
    bmp180_readRegister(OUT_MSB, &ut_data[0]);
    bmp180_readRegister(OUT_MSB + 1, &ut_data[1]);

    return (convert8bitto16bit(ut_data[0], ut_data[1]));
}

/**
 * @brief init I2C for bmp180
 *
 * @return int return error code
 */
int bmp180_begin() {
    // Set up the I2C interface
    i2c = device_get_binding("I2C_0");
    if (i2c == NULL) {
        printk("Error acquiring i2c1 interface\n");
        return -1;
    }
    bmp180_writeRegister(0xE0, 0x86);
    bmp180_readRegister(0xD0, &id);
    printk(" id : %02x : ", id);
    switch (id) {
    case 0x55:
        printk(" BMP180\r\n");
        break;
    case 0x58:
        printk(" BMP280\r\n");
        break;
    default:
        printk(" UNKNOWN\r\n");
        break;
    }

    bmp180_readCalibrationData();
    return 0;
}

/**
 * @brief read temperature from bmp180
 *
 * @return int32_t temperature
 */
int32_t bmp180_readTemperature() {
    int16_t ut = bmp180_read_ut();
    int32_t X1, X2;

    X1 = (ut - bmp180.AC6) * bmp180.AC5 / powerof2(15);
    X2 = bmp180.MC * powerof2(11) / (X1 + bmp180.MD);
    bmp180.B5 = X1 + X2;
    bmp180.temperature = ((bmp180.B5 + 8) / powerof2(4)) / 10.0;
    return ((int32_t) bmp180.temperature * 100);
}

/**
 * @brief read pressure from bmp180
 *
 * @return int32_t pressure
 */
int32_t bmp180_readPressure() {
    int32_t X1, X2, X3, up = bmp180_read_up(bmp180), p;
    bmp180.B6 = bmp180.B5 - 4000;
    X1 = (bmp180.B2 * (bmp180.B6 * bmp180.B6 / powerof2(12))) / powerof2(11);
    X2 = bmp180.AC2 * bmp180.B6 / powerof2(11);
    X3 = X1 + X2;
    bmp180.B3 = (((bmp180.AC1 * 4 + X3) << bmp180.oss) + 2) / 4;
    X1 = bmp180.AC3 * bmp180.B6 / powerof2(13);
    X2 = (bmp180.B1 * (bmp180.B6 * bmp180.B6 / powerof2(12))) / powerof2(16);
    X3 = ((X1 + X2) + 2) / powerof2(2);
    bmp180.B4 = bmp180.AC4 * (uint32_t) (X3 + 32768) / powerof2(15);
    bmp180.B7 = ((uint32_t) up - bmp180.B3) * (50000 >> bmp180.oss);
    if (bmp180.B7 < 0x80000000) {
        p = (bmp180.B7 * 2) / bmp180.B4;
    } else {
        p = (bmp180.B7 / bmp180.B4) * 2;
    }
    X1 = (p / powerof2(8)) * (p / powerof2(8));
    X1 = (X1 * 3038) / powerof2(16);
    X2 = (-7357 * p) / powerof2(16);
    p = p + (X1 + X2 + 3791) / powerof2(4);
    bmp180.pressure = p;
    return ((uint32_t) bmp180.pressure);
}
