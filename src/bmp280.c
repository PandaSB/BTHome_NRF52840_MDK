/** @file
 *  @brief Bmp280 Service code
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bmp280.h"
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <drivers/spi.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/printk.h>

static uint8_t id = 0;
static int32_t t_fine;
static uint16_t dig_T1;   // calibration for temperature
static int16_t dig_T2;    // calibration for temperature
static int16_t dig_T3;    // calibration for temperature
static uint16_t dig_P1;   // calibration for pressure
static int16_t dig_P2;    // calibration for pressure
static int16_t dig_P3;    // calibration for pressure
static int16_t dig_P4;    // calibration for pressure
static int16_t dig_P5;    // calibration for pressure
static int16_t dig_P6;    // calibration for pressure
static int16_t dig_P7;    // calibration for pressure
static int16_t dig_P8;    // calibration for pressure
static int16_t dig_P9;    // calibration for pressure
// "private" functions for use within this module only
int readRegister(uint8_t RegNum, uint8_t *Value);
int writeRegister(uint8_t RegNum, uint8_t Value);
void readCalibrationData();
// static const struct spi_config * cfg;
static const struct device *i2c;

/**
 * @brief Init I2c from bmp280
 *
 * @return int
 */
int bmp280_begin() {
    // Set up the I2C interface
    i2c = device_get_binding("I2C_0");
    if (i2c == NULL) {
        printk("Error acquiring i2c1 interface\n");
        return -1;
    }
    bmp280_writeRegister(0xE0, 0x86);
    bmp280_readRegister(0xD0, &id);
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

    bmp280_readCalibrationData();
    return 0;
}

/**
 * @brief Read pressure from bmp280
 *
 * @return int32_t pressure from bmp280
 */
int32_t bmp280_readPressure()   // returns Pressure * 100
{
    bmp280_readTemperature();   // Pressure reading needs the temperature reading to
                                // calculate pressure
    uint8_t PressureMSB, PressureLSB, PressureXLSB;
    bmp280_readRegister(0xF7, &PressureMSB);
    bmp280_readRegister(0xF8, &PressureLSB);
    bmp280_readRegister(0xF9, &PressureXLSB);
    // Convert temperature data bytes to 20-bits within 32 bit integer
    uint32_t adc_P = (((uint32_t) PressureMSB << 16) + ((uint32_t) PressureLSB << 8) + ((uint32_t) PressureXLSB)) >> 4;
    int32_t var1, var2;
    uint32_t p;
    var1 = (((int32_t) t_fine) >> 1) - (int32_t) 64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t) dig_P6);
    var2 = var2 + ((var1 * ((int32_t) dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t) dig_P4) << 16);
    var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t) dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t) dig_P1)) >> 15);
    if (var1 == 0) {
        return 0;
    }
    p = (((uint32_t) (((int32_t) 1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000) {
        p = (p << 1) / ((uint32_t) var1);
    } else {
        p = (p / (uint32_t) var1) * 2;
    }
    var1 = (((int32_t) dig_P9) * ((int32_t) (((p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t) (p >> 2)) * ((int32_t) dig_P8)) >> 13;
    p = (uint32_t) ((int32_t) p + ((var1 + var2 + dig_P7) >> 4));
    return p;
}

/**
 * @brief read temperature from bmp280
 *
 * @return int32_t temperature from bmp280
 */
int32_t bmp280_readTemperature()   // returns Temperature * 100
{
    int32_t var1, var2, T;
    // function to read the temperature in BMP280, calibrate it and return the
    // value in Celsius
    bmp280_writeRegister(0xF4,
                         0x2E);   // Initializing the write register for temperature
    uint8_t status = 0x08;
    // Start the i2c transmission
    while (status & 0x08) {
        bmp280_readRegister(0xF3, &status);
    }
    // Temperature reading transmitted in 3 parts - MSB, LSB & XLSB
    uint8_t TemperatureMSB, TemperatureLSB, TemperatureXLSB;
    bmp280_readRegister(0xFA, &TemperatureMSB);
    bmp280_readRegister(0xFB, &TemperatureLSB);
    bmp280_readRegister(0xFC, &TemperatureXLSB);

    // Convert temperature data bytes to 20-bits within 32 bit integer
    int32_t adc_T = (((long) TemperatureMSB << 16) + ((long) TemperatureLSB << 8) + (long) TemperatureXLSB) >> 4;

    var1 = ((((adc_T >> 3) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t) dig_T1)) * ((adc_T >> 4) - ((int32_t) dig_T1))) >> 12) * ((int32_t) dig_T3)) >> 14;
    t_fine = var1 + var2;   // store to static (private to this module) variable so
                            // that pressure routine can access temperature data
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

/**
 * @brief Read register from bmp280
 *
 * @param RegNum Register index
 * @param Value pointer on value of register
 * @return int error code
 */
int bmp280_readRegister(uint8_t RegNum, uint8_t *Value) {
    // reads a series of bytes, starting from a specific register
    int nack;
    nack = i2c_reg_read_byte(i2c, BMP280_R_ADDRESS, RegNum, Value);
    return nack;
}

/**
 * @brief Write register on vmp280
 *
 * @param RegNum register index
 * @param Value pointer on register value
 * @return int error code
 */
int bmp280_writeRegister(uint8_t RegNum, uint8_t Value) {
    // sends a byte to a specific register
    uint8_t Buffer[2];
    Buffer[0] = Value;
    int nack;
    nack = i2c_reg_write_byte(i2c, BMP280_R_ADDRESS, RegNum, Value);
    return nack;
}

/**
 * @brief Read calibration data
 *
 */
void bmp280_readCalibrationData() {
    uint8_t CalibrationData[26];
    int i;
#define CAL_START 0x88   // Defining the address where the calibration data should start
    for (i = 0; i < 26; i++) {
        // Read calibration table
        bmp280_readRegister(CAL_START + i, &CalibrationData[i]);
    }
    dig_T1 = ((uint16_t) CalibrationData[1] << 8) + (uint16_t) (CalibrationData[0]);
    dig_T2 = ((uint16_t) CalibrationData[3] << 8) + (uint16_t) (CalibrationData[2]);
    dig_T3 = ((uint16_t) CalibrationData[5] << 8) + (uint16_t) (CalibrationData[4]);
    dig_P1 = ((uint16_t) CalibrationData[7] << 8) + (uint16_t) (CalibrationData[6]);
    dig_P2 = ((uint16_t) CalibrationData[9] << 8) + (uint16_t) (CalibrationData[8]);
    dig_P3 = ((uint16_t) CalibrationData[11] << 8) + (uint16_t) (CalibrationData[10]);
    dig_P4 = ((uint16_t) CalibrationData[13] << 8) + (uint16_t) (CalibrationData[12]);
    dig_P5 = ((uint16_t) CalibrationData[15] << 8) + (uint16_t) (CalibrationData[14]);
    dig_P6 = ((uint16_t) CalibrationData[17] << 8) + (uint16_t) (CalibrationData[16]);
    dig_P7 = ((uint16_t) CalibrationData[19] << 8) + (uint16_t) (CalibrationData[18]);
    dig_P8 = ((uint16_t) CalibrationData[21] << 8) + (uint16_t) (CalibrationData[20]);
    dig_P9 = ((uint16_t) CalibrationData[23] << 8) + (uint16_t) (CalibrationData[22]);
}
