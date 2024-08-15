/*
 * Copyright (c) 2007-2009, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _PROC_COMM_CLIENTS_H_
#define _PROC_COMM_CLIENTS_H_


/*HSUSB related*/
void pcom_usb_vbus_power(int state);
void pcom_usb_reset_phy(void);
void pcom_enable_hsusb_clk(void);
void pcom_disable_hsusb_clk(void);

/*UART related*/
UINT32 pcom_get_uart_clk(int uart_base_addr);

/*SDC related*/
void pcom_sdcard_gpio_config(int instance);
void pcom_sdcard_power(int state);
void pcom_set_sdcard_clk(int instance, int rate);
UINT32 pcom_get_sdcard_clk(int instance);
void pcom_enable_sdcard_clk(int instance);
void pcom_disable_sdcard_clk(int instance);
void pcom_enable_sdcard_pclk(int instance);
void pcom_disable_sdcard_pclk(int instance);
UINT32 pcom_is_sdcard_clk_enabled(int instance);
UINT32 pcom_is_sdcard_pclk_enabled(int instance);

/*LCDC related*/
void pcom_set_lcdc_clk(int rate);
void pcom_enable_lcdc_pad_clk(void);
void pcom_enable_lcdc_clk(void);
UINT32 pcom_get_lcdc_clk(void);

void pcom_end_cmds(void);

#endif /*_PROC_COMM_CLIENTS_H*/
