/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "kd_imgsensor.h"

#include "regulator/regulator.h"
#include "gpio/gpio.h"
#include "mclk/mclk.h"

#include "imgsensor_cfg_table.h"

enum IMGSENSOR_RETURN
	(*hw_open[IMGSENSOR_HW_ID_MAX_NUM])(struct IMGSENSOR_HW_DEVICE **) = {
	imgsensor_hw_regulator_open,
	imgsensor_hw_gpio_open,
	imgsensor_hw_mclk_open
};

/*
 * OPPO 18511/18510/18121 (A5s) camera power routing.
 *
 * The A5s uses four camera slots: main (rear), sub (front), main2 (rear
 * auxiliary) and sub2 (front auxiliary, unused).  The I2C SCL/SDA pinctrl
 * states of the original 4.9.80 tree do not exist in the CPH2185 camera
 * driver, so the corresponding steps are dropped here.
 */
struct IMGSENSOR_HW_CFG imgsensor_custom_config[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	/* Slots not wired on the A5s, kept so that imgsensor_hw_init()
	 * can scan every IMGSENSOR_SENSOR_IDX_* value.
	 */
	{
		IMGSENSOR_SENSOR_IDX_MAIN3,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN4,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};

/* The common code always references the yogurt table; the A5s has its own
 * routing above, so keep a identical table here to satisfy the reference.
 */
struct IMGSENSOR_HW_CFG imgsensor_custom_config_yogurt[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN3,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN4,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};

struct IMGSENSOR_HW_POWER_SEQ platform_power_sequence[] = {
	{NULL}
};

/* Legacy design */
struct IMGSENSOR_HW_POWER_SEQ sensor_power_sequence[] = {
#if defined(S5K3L6_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3L6_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1100, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(HI1336_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI1336_MIPI_RAW,
		{
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1100, 5},
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_Low, 10},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(HI1336_LIANYI_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI1336_LIANYI_MIPI_RAW,
		{
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1100, 5},
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_Low, 10},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(GC8034_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC8034_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 1},
			{PDN, Vol_1200, 0},
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(HI846_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI846_MIPI_RAW,
		{
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{PDN, Vol_1200, 0},
			{DVDD, Vol_1200, 1},
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(HI846_DONGCI_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI846_DONGCI_MIPI_RAW,
		{
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{PDN, Vol_1200, 0},
			{DVDD, Vol_1200, 1},
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(S5K3P9SP_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3P9SP_MIPI_RAW,
		{
			{RST, Vol_Low, 1},
			{SensorMCLK, Vol_High, 1},
			{AVDD, Vol_2800, 1},
			{PDN, Vol_1100, 0},
			{DVDD, Vol_1100, 1},
			{DOVDD, Vol_1800, 1},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(SP2509V_MIPI_RAW)
	{
		SENSOR_DRVNAME_SP2509V_MIPI_RAW,
		{
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 5},
			{PDN, Vol_High, 0, Vol_High, 0},
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5, Vol_Low, 0},
			{RST, Vol_High, 5, Vol_Low, 0}
		},
	},
#endif
#if defined(GC2375H_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC2375H_MIPI_RAW,
		{
			{PDN, Vol_High, 0, Vol_Low, 0},
			{DOVDD, Vol_1800, 1, Vol_Low, 5},
			{AVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_Low, 0, Vol_Low, 0},
			{PDN, Vol_Low, 0, Vol_High, 0},
			{RST, Vol_High, 0, Vol_High, 0}
		},
	},
#endif
#if defined(GC2375H_LIANYI_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC2375H_LIANYI_MIPI_RAW,
		{
			{PDN, Vol_High, 0, Vol_Low, 0},
			{DOVDD, Vol_1800, 1, Vol_Low, 5},
			{AVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_Low, 0, Vol_Low, 0},
			{PDN, Vol_Low, 0, Vol_High, 0},
			{RST, Vol_High, 0, Vol_High, 0}
		},
	},
#endif

	{NULL}
};

int yogurt_project(void)
{
	/* The A5s uses imgsensor_custom_config, not the yogurt tables. */
	return 0;
}
