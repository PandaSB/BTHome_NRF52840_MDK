
/** @file
 *  @brief I2C Service code 
 */

/*
 * Copyright (c) 2023 BARTHELEMY Stéphane 
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include <logging/log.h>
#include <drivers/i2c.h>
#include <sys/byteorder.h>
#include <sys/util.h>


void i2c_scan(void) {
    const struct device *i2c_dev;
    int error;
    int cnt = 0 ; 

    i2c_dev = device_get_binding("I2C_0");
    if (!i2c_dev) {
        printk("Binding failed.");
        return;
    }

    /* Demonstration of runtime configuration */
    i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD));
    printk("Value of NRF_TWIM2->PSEL.SCL : %d \n",NRF_TWIM0->PSEL.SCL);
    printk("Value of NRF_TWIM2->PSEL.SDA : %d \n",NRF_TWIM0->PSEL.SDA);
    printk("Value of NRF_TWIM2->FREQUENCY: %d \n",NRF_TWIM0->FREQUENCY);
    printk("26738688 -> 100k\n");

    for (uint8_t i = 4; i <= 0x7F; i++) {
        struct i2c_msg msgs[1];
        uint8_t dst = 1;

        msgs[0].buf = &dst;
        msgs[0].len = 1U;
        msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

        error = i2c_transfer(i2c_dev, &msgs[0], 1, i);
        if (error == 0) {
            printk("0x%2x FOUND\n", i);
            cnt++;
        }
        else {
            //printk("error %d \n", error);
        }

    }
    printk (" -- %d devices \n", cnt) ; 
}

