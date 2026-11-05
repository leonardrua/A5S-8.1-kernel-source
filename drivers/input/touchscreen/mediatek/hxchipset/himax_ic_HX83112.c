/* SPDX-License-Identifier: GPL-2.0 */
/* Himax Android Driver Sample Code for HX83112 chipset. */

#include "himax_ic_HX83112.h"
#include "himax_modular.h"

static void hx83112_chip_init(void)
{
	(*kp_private_ts)->chip_cell_type = CHIP_IS_IN_CELL;
	(*kp_IC_CHECKSUM) = HX_TP_BIN_CHECKSUM_CRC;

	/* HX83112 firmware version and configuration addresses. */
	(*kp_FW_VER_MAJ_FLASH_ADDR) = 0x00c005;
	(*kp_FW_VER_MIN_FLASH_ADDR) = 0x00c006;
	(*kp_CFG_VER_MAJ_FLASH_ADDR) = 0x00c100;
	(*kp_CFG_VER_MIN_FLASH_ADDR) = 0x00c101;
	(*kp_CID_VER_MAJ_FLASH_ADDR) = 0x00c002;
	(*kp_CID_VER_MIN_FLASH_ADDR) = 0x00c003;
	(*kp_CFG_TABLE_FLASH_ADDR) = 0x10000;

	I("%s: IC cell type = %d\n", __func__,
		(*kp_private_ts)->chip_cell_type);
}

static bool hx83112_sense_off(bool check_en)
{
	uint8_t cnt = 0;
	uint8_t tmp_data[DATA_LEN_4];
	int ret;

	do {
		tmp_data[0] = (*kp_pic_op)->data_i2c_psw_lb[0];
		ret = kp_himax_bus_write((*kp_pic_op)->adr_i2c_psw_lb[0],
			tmp_data, 1, HIMAX_I2C_RETRY_TIMES);
		if (ret < 0)
			goto i2c_error;

		tmp_data[0] = (*kp_pic_op)->data_i2c_psw_ub[0];
		ret = kp_himax_bus_write((*kp_pic_op)->adr_i2c_psw_ub[0],
			tmp_data, 1, HIMAX_I2C_RETRY_TIMES);
		if (ret < 0)
			goto i2c_error;

		/* HX83112 strobes the low-byte password twice. */
		tmp_data[0] = 0x00;
		ret = kp_himax_bus_write((*kp_pic_op)->adr_i2c_psw_lb[0],
			tmp_data, 1, HIMAX_I2C_RETRY_TIMES);
		if (ret < 0)
			goto i2c_error;

		tmp_data[0] = (*kp_pic_op)->data_i2c_psw_lb[0];
		ret = kp_himax_bus_write((*kp_pic_op)->adr_i2c_psw_lb[0],
			tmp_data, 1, HIMAX_I2C_RETRY_TIMES);
		if (ret < 0)
			goto i2c_error;

		tmp_data[0] = (*kp_pic_op)->data_i2c_psw_ub[0];
		ret = kp_himax_bus_write((*kp_pic_op)->adr_i2c_psw_ub[0],
			tmp_data, 1, HIMAX_I2C_RETRY_TIMES);
		if (ret < 0)
			goto i2c_error;

		ret = kp_g_core_fp->fp_register_read(
			(*kp_pic_op)->addr_cs_central_state, ADDR_LEN_4,
			tmp_data, 0);
		if (ret < 0)
			goto i2c_error;

		I("%s: safe-mode state = 0x%02X\n", __func__, tmp_data[0]);
		if (tmp_data[0] == 0x0c) {
			ret = kp_g_core_fp->fp_register_write(
				(*kp_pic_op)->addr_tcon_on_rst, DATA_LEN_4,
				(*kp_pic_op)->data_rst, 0);
			if (ret < 0)
				goto i2c_error;
			usleep_range(1000, 1100);

			tmp_data[3] = (*kp_pic_op)->data_rst[3];
			tmp_data[2] = (*kp_pic_op)->data_rst[2];
			tmp_data[1] = (*kp_pic_op)->data_rst[1];
			tmp_data[0] = (*kp_pic_op)->data_rst[0] | 0x01;
			ret = kp_g_core_fp->fp_register_write(
				(*kp_pic_op)->addr_tcon_on_rst, DATA_LEN_4,
				tmp_data, 0);
			if (ret < 0)
				goto i2c_error;

			ret = kp_g_core_fp->fp_register_write(
				(*kp_pic_op)->addr_adc_on_rst, DATA_LEN_4,
				(*kp_pic_op)->data_rst, 0);
			if (ret < 0)
				goto i2c_error;
			usleep_range(1000, 1100);

			tmp_data[3] = (*kp_pic_op)->data_rst[3];
			tmp_data[2] = (*kp_pic_op)->data_rst[2];
			tmp_data[1] = (*kp_pic_op)->data_rst[1];
			tmp_data[0] = (*kp_pic_op)->data_rst[0] | 0x01;
			ret = kp_g_core_fp->fp_register_write(
				(*kp_pic_op)->addr_adc_on_rst, DATA_LEN_4,
				tmp_data, 0);
			if (ret < 0)
				goto i2c_error;

			return true;
		}

		usleep_range(10000, 11000);
#if defined(HX_RST_PIN_FUNC)
		kp_g_core_fp->fp_ic_reset(false, false);
#endif
	} while (check_en && cnt++ < 15);

	return false;

i2c_error:
	E("%s: I2C/register access failed (%d)\n", __func__, ret);
	return false;
}

static bool hx83112_chip_detect(void)
{
	uint8_t tmp_data[DATA_LEN_4];
	int ret;
	int i;

	if (himax_ic_setup_external_symbols())
		return false;

	ret = kp_himax_mcu_in_cmd_struct_init();
	if (ret < 0) {
		E("%s: command structure initialization failed (%d)\n",
			__func__, ret);
		return false;
	}
	kp_himax_mcu_in_cmd_init();

	if (!hx83112_sense_off(true)) {
		E("%s: failed to enter safe mode\n", __func__);
		return false;
	}

	for (i = 0; i < 5; i++) {
		ret = kp_g_core_fp->fp_register_read(
			(*kp_pfw_op)->addr_icid_addr, DATA_LEN_4,
			tmp_data, 0);
		if (ret < 0) {
			E("%s: failed to read IC ID (%d)\n", __func__, ret);
			continue;
		}

		I("%s: IC ID = %02X %02X %02X\n", __func__, tmp_data[3],
			tmp_data[2], tmp_data[1]);
		if (tmp_data[3] != 0x83 || tmp_data[2] != 0x11 ||
			(tmp_data[1] != 0x2a && tmp_data[1] != 0x2b)) {
			E("%s: HX83112 ID did not match\n", __func__);
			continue;
		}

		if (tmp_data[1] == 0x2a) {
			strlcpy((*kp_private_ts)->chip_name,
				HX_83112A_SERIES_PWON,
				sizeof((*kp_private_ts)->chip_name));
		} else {
			strlcpy((*kp_private_ts)->chip_name,
				HX_83112B_SERIES_PWON,
				sizeof((*kp_private_ts)->chip_name));
		}

		(*kp_ic_data)->ic_adc_num = hx83112_data_adc_num;
		kp_g_core_fp->fp_chip_init = hx83112_chip_init;
		kp_g_core_fp->fp_sense_off = hx83112_sense_off;
		I("%s: detected %s\n", __func__,
			(*kp_private_ts)->chip_name);
		return true;
	}

	return false;
}

DECLARE(HX_MOD_KSYM_HX83112);

static int himax_hx83112_probe(void)
{
	himax_add_chip_dt(hx83112_chip_detect);
	return 0;
}

static int himax_hx83112_remove(void)
{
	free_chip_dt_table();
	return 0;
}

static int __init himax_hx83112_init(void)
{
	return himax_hx83112_probe();
}

static void __exit himax_hx83112_exit(void)
{
	himax_hx83112_remove();
}

module_init(himax_hx83112_init);
module_exit(himax_hx83112_exit);

MODULE_DESCRIPTION("HIMAX HX83112 touch driver");
MODULE_LICENSE("GPL");
