/** @file
 *  @brief am2320 header
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AM2320_H_
#define _AM2320_H_

#include <stdint.h>
#define AM2320_R_ADDRESS (0x5C)

#define AM2320_CMD_READREG 0x03   ///< read register command
#define AM2320_REG_TEMP_H  0x02   ///< temp register address

#define AM2320_REG_HUM_H 0x00   ///< humidity register address

#define NAN (-1000)

int am2320_begin();
int32_t am2320_readTemperature();   // returns Temperature * 100
int32_t am2320_readHumidity();      // returns Temperature * 100

#endif