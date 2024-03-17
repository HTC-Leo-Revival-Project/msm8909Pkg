/* board-htcleo.h
 *
 * Copyright (C) 2012 Shantanu Gupta <shans95g@gmail.com>
 * Copyright (C) 2011 Alexander Tarasikov <alexander.tarasikov@gmail.com>
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

#ifndef __HTCLEO_H__
#define __HTCLEO_H__

#define GPIO_I2C_CLK				95
#define GPIO_I2C_DAT				96
#define HTCLEO_GPIO_UP_INT_N		90
#define HTCLEO_GPIO_UP_RESET_N		91
#define HTCLEO_GPIO_LS_EN_N			119
#define DS2746_I2C_SLAVE_ADDR		0x26

#define MSM_PANEL_PHYS        		0x00080000
/* Display */
#define HTCLEO_GPIO_LCM_POWER		88
#define HTCLEO_GPIO_LCM_RESET		29
#define HTCLEO_LCD_R1				(114)
#define HTCLEO_LCD_R2				(115)
#define HTCLEO_LCD_R3				(116)
#define HTCLEO_LCD_R4				(117)
#define HTCLEO_LCD_R5				(118)
#define HTCLEO_LCD_G0				(121)
#define HTCLEO_LCD_G1				(122)
#define HTCLEO_LCD_G2				(123)
#define HTCLEO_LCD_G3				(124)
#define HTCLEO_LCD_G4				(125)
#define HTCLEO_LCD_G5				(126)
#define HTCLEO_LCD_B1				(130)
#define HTCLEO_LCD_B2				(131)
#define HTCLEO_LCD_B3				(132)
#define HTCLEO_LCD_B4				(133)
#define HTCLEO_LCD_B5				(134)
#define HTCLEO_LCD_PCLK	       		(135)
#define HTCLEO_LCD_VSYNC	      	(136)
#define HTCLEO_LCD_HSYNC			(137)
#define HTCLEO_LCD_DE				(138)

/* Touchscreen */
#define HTCLEO_GPIO_TS_POWER			160
#define HTCLEO_GPIO_TS_IRQ      		92
#define HTCLEO_GPIO_TS_SEL      		108
#define HTCLEO_GPIO_TS_MULT				82

#define HTCLEO_GPIO_H2W_CLK				27
#define HTCLEO_GPIO_LED_3V3_EN			85
#define HTCLEO_GPIO_KP_MKOUT0    		33
#define HTCLEO_GPIO_KP_MKOUT1    		32
#define HTCLEO_GPIO_KP_MKOUT2    		31
#define HTCLEO_GPIO_KP_MPIN0     		42
#define HTCLEO_GPIO_KP_MPIN1     		41
#define HTCLEO_GPIO_KP_MPIN2     		40
#define HTCLEO_GPIO_KP_LED	 			48
#define HTCLEO_GPIO_POWER_KEY			94
#define HTCLEO_GPIO_VIBRATOR_ON        	100
#define HTCLEO_GPIO_FLASHLIGHT_TORCH   	143

/* Battery */
#define HTCLEO_GPIO_BATTERY_CHARGER_ENABLE	22
#define HTCLEO_GPIO_BATTERY_CHARGER_CURRENT	16
#define HTCLEO_GPIO_BATTERY_OVER_CHG		147
#define HTCLEO_GPIO_POWER_USB     			109
#define HTCLEO_GPIO_USBPHY_3V3_ENABLE		104

//extern struct board_mach htcleo;

/* htcleo_board_functions */
void msm_i2c_init(void);
void msm_microp_i2c_init(void);
void msm_prepare_clocks(void);
void htcleo_display_init(void);
void htcleo_display_shutdown(void);
void htcleo_usb_init(void);
void htcleo_udc_init(void);
void htcleo_keypad_init(void);
void htcleo_flash_info_init(void);
void htcleo_devinfo_init(void);
void htcleo_ptable_init(void);
void htcleo_ptable_re_init(void);
void htcleo_acpu_clock_init(void);
void htcleo_prepare_for_linux(void);
int  htcleo_mmc_init(void *arg);
void htcleo_ts_init(void);
void htcleo_ts_deinit(void);
void htcleo_ls_init(void);
void htcleo_vibrator(int enable);
void htcleo_vibrate_once(void);
void htcleo_key_bkl_pwr(int enable);
int htcleo_notif_led_set_mode(void *mode);
void htcleo_panel_set_brightness(int val);
unsigned htcleo_boot_mark;
unsigned htcleo_reboot_mode;
int htcleo_notif_led_mode;
int htcleo_chg_voltage_threshold;
int htcleo_pause_for_battery_charge;
int htcleo_suspend(void *arg);

#endif // __HTCLEO_H__
