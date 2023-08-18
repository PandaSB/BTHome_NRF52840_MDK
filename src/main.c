/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <pm/pm.h>
#include <pm/device.h>

#include <i2c.h>
#include <bmp180.h>
#include <am2320.h>

#define SERVICE_DATA_LEN        16
#define SERVICE_UUID            0xfcd2      /* BTHome service UUID */
#define IDX_TEMPL               4           /* Index of lo byte of temp in service data*/
#define IDX_TEMPH               5           /* Index of hi byte of temp in service data*/
#define IDX_PRESL               7           /* Index of lo byte of temp in service data*/
#define IDX_PRESM               8           /* Index of hi byte of temp in service data*/
#define IDX_PRESH               9           /* Index of hi byte of temp in service data*/
#define IDX_TEMPL2              11           /* Index of lo byte of temp in service data*/
#define IDX_TEMPH2              12           /* Index of hi byte of temp in service data*/
#define IDX_HUML                14           /* Index of lo byte of temp in service data*/
#define IDX_HULH                15           /* Index of hi byte of temp in service data*/

#define ADV_PARAM BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
				  BT_GAP_ADV_SLOW_INT_MIN, \
				  BT_GAP_ADV_SLOW_INT_MAX, NULL)


static uint8_t service_data[SERVICE_DATA_LEN] = {
	BT_UUID_16_ENCODE(SERVICE_UUID),
	0x40,
	0x02,	/* Temperature */
	0xc4,	/* Low byte */
	0x00,   /* High byte */
	0x04,	/* Pressure */
	0x00,	/*  low byte*/
	0x00,   /*  middle byte*/
    0x00,   /*  high byte*/
    0x02,	/* Temperature */
	0xc4,	/* Low byte */
	0x00,   /* High byte */
    0x03,	/* humidity */
	0xc4,	/* Low byte */
	0x00,   /* High byte */
};

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
}

int main(void)
{
	int err;
	int temp = 0;
    int temp2 = 0;
    int humidity = 0 ;
    int pressure = 0 ; 

	printk("Starting BTHome sensor template\n");

    bmp180_begin() ; 
    am2320_begin() ;

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	for (;;) {
        pressure = bmp180_readPressure() ; 
        temp = bmp180_readTemperature() ;
        temp2 = am2320_readTemperature();
        humidity = am2320_readHumidity() ;
        printk ("temperature   : %u\n", temp) ; 
        printk ("pression      : %u\n", pressure) ; 
        printk ("temperature 2 : %u\n", temp2) ; 
        printk ("humidity      : %u\n", humidity) ; 

		service_data[IDX_TEMPH] = (temp) >> 8;
		service_data[IDX_TEMPL] = (temp) & 0xff;

        service_data[IDX_PRESH] = ((pressure) >> 16) & 0xff;
		service_data[IDX_PRESM] = ((pressure) >> 8 ) & 0xff;;
		service_data[IDX_PRESL] = (pressure) & 0xff;
	
		service_data[IDX_TEMPH2] = (temp2) >> 8;
		service_data[IDX_TEMPL2] = (temp2) & 0xff;

        service_data[IDX_HULH] = (humidity) >> 8;
		service_data[IDX_HUML] = (humidity) & 0xff;

		err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		if (err) {
			printk("Failed to update advertising data (err %d)\n", err);
		}
		k_sleep(K_MSEC(BT_GAP_ADV_SLOW_INT_MIN));
	}
	return 0;
}