/** @file
 *  @brief am2320 service
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <am2320.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <drivers/spi.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/printk.h>

static const struct device *i2c_am2320;
/**
 * @brief Calcul crc16
 *
 * @param buffer input buffer
 * @param nbytes buffer size
 * @return uint16_t checksulm
 */
uint16_t am2320_crc16(uint8_t *buffer, uint8_t nbytes) {
    uint16_t crc = 0xffff;
    for (int i = 0; i < nbytes; i++) {
        uint8_t b = buffer[i];
        crc ^= b;
        for (int x = 0; x < 8; x++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
/**
 * @brief Read uint16 register from am2330
 *
 * @param RegNum Index of register
 * @param Value pointer to returned Value
 * @return int error code
 */
int am2320_readRegister16(uint8_t RegNum, uint16_t *Value) {
    // reads a series of bytes, starting from a specific register
    int nack;
    uint8_t in_buf[6] = {0};
    uint8_t out_buf[6] = {0};
    uint16_t the_crc;
    uint16_t calc_crc;

    nack = i2c_write(i2c_am2320, in_buf, 1, AM2320_R_ADDRESS);
    k_msleep(10);   // wait 10 ms

    in_buf[0] = AM2320_CMD_READREG;
    in_buf[1] = RegNum;
    in_buf[2] = 2;   // 2 bytes
    in_buf[3] = 0;
    in_buf[4] = 0;
    in_buf[5] = 0;
    // nack=i2c_reg_read_byte(i2c_am2320,AM2320_R_ADDRESS,RegNum,Value);
    nack = i2c_write(i2c_am2320, in_buf, 3, AM2320_R_ADDRESS);
    k_msleep(2);
    nack = i2c_read(i2c_am2320, out_buf, 6, AM2320_R_ADDRESS);
    if (out_buf[0] != 0x03) {
        *Value = 0xFFFF;
        return nack;   // must be 0x03 modbus reply
    }
    if (out_buf[1] != 2) {
        *Value = 0xFFFF;
        return nack;   // must be 2 bytes reply
    }
    the_crc = (out_buf[5] << 8) | out_buf[4];
    calc_crc = am2320_crc16(out_buf, 4);   // preamble + data
    printk("CRC: %02x \r\n", calc_crc);

    if (the_crc != calc_crc) {
        *Value = 0xFFFF;
        return nack;
    }
    *Value = (out_buf[2] << 8) | out_buf[3];
    return nack;
}
/**
 * @brief Write uint16 register ( not coded)
 *
 * @param RegNum index of register
 * @param Value pointer to register value
 * @return int error code
 */
int am2320_writeRegister(uint8_t RegNum, uint8_t Value) {
    int nack = -1;
    return nack;
}

/**
 * @brief Init I2c and calibration for am2320
 *
 * @return int error code
 */
int am2320_begin() {
    // Set up the I2C interface
    i2c_am2320 = device_get_binding("I2C_0");
    if (i2c_am2320 == NULL) {
        printk("Error acquiring i2c0 interface\n");
        return -1;
    }
    printk("init i2c0 for am2320\r\n");
    return 0;
}

/**
 * @brief Read temperature from am2320
 *
 * @return int32_t temperature
 */
int32_t am2320_readTemperature()   // returns Temperature * 100
{
    uint16_t t;
    float ft;
    am2320_readRegister16(AM2320_REG_TEMP_H, &t);
    if (t == 0xFFFF)
        return NAN;
    // check sign bit - the temperature MSB is signed , bit 0-15 are magnitude
    if (t & 0x8000) {
        ft = -(int16_t) (t & 0x7fff);
    } else {
        ft = (int16_t) t;
    }
    return ft * 10;
}

/**
 * @brief return humidity from am2320
 *
 * @return int32_t humidity value
 */
int32_t am2320_readHumidity()   // returns Temperature * 100
{
    uint16_t h;
    am2320_readRegister16(AM2320_REG_HUM_H, &h);

    if (h == 0xFFFF)
        return NAN;

    return (h * 10);
}