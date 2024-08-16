/* linux/arch/arm/mach-msm/board-vivo.h
 * Copyright (C) 2007-2009 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#ifndef __ARCH_ARM_MACH_MSM_BOARD_VISION_H
#define __ARCH_ARM_MACH_MSM_BOARD_VISION_H

#define SAMSUNG_PANEL           0
#define SAMSUNG_PANELII         1
/*Bitwise mask for SONY PANEL ONLY*/
#define SONY_PANEL              0x1             /*Set bit 0 as 1 when it is SONY PANEL*/
#define SONY_PWM_SPI            0x2             /*Set bit 1 as 1 as PWM_SPI mode, otherwise it is PWM_MICROP mode*/
#define SONY_GAMMA              0x4             /*Set bit 2 as 1 when panel contains GAMMA table in its NVM*/
#define SONY_RGB666             0x8             /*Set bit 3 as 1 when panel is 18 bit, otherwise it is 16 bit*/
#define SONY_PANEL_SPI           (SONY_PANEL | SONY_PWM_SPI | SONY_GAMMA | SONY_RGB666)

extern struct platform_device msm_device_mdp;
extern struct platform_device msm_device_mddi0;

/* Macros assume PMIC GPIOs start at 0 */
#define PM8058_GPIO_PM_TO_SYS(pm_gpio)     (pm_gpio + NR_GPIO_IRQS)
// 182 + x
#define PM8058_GPIO_SYS_TO_PM(sys_gpio)    (sys_gpio - NR_GPIO_IRQS)
#define PM8058_MPP_BASE			   PM8058_GPIO_PM_TO_SYS(PM8058_GPIOS)
#define PM8058_MPP_PM_TO_SYS(pm_gpio)	   (pm_gpio + PM8058_MPP_BASE)
#define PM8058_uP_PM_TO_SYS(pm_gpio)	   (pm_gpio + FIRST_BOARD_IRQ + NR_BOARD_IRQS)

#define MSM_LINUX_BASE1		0x04000000
#define MSM_LINUX_SIZE1		0x0C000000
#define MSM_LINUX_BASE2		0x20000000
#define MSM_LINUX_SIZE2		0x10000000

#define MSM_RAM_CONSOLE_BASE	0x00500000
#define MSM_RAM_CONSOLE_SIZE	0x00100000

#define MSM_PMEM_ADSP_SIZE	0x01E00000

#define MSM_PMEM_AUDIO_SIZE	0x00000000

#define PMEM_KERNEL_EBI0_SIZE   0x00500000

#define MSM_PMEM_SF_SIZE	0x01F00000

#define MSM_FB_SIZE    roundup((800 * ALIGN(480, 32) * 4 * 3), 4096) /* 4 bpp x 3 pages, Note: must be multiple of 4096 */

#ifdef CONFIG_ION_MSM
#define MSM_ION_CAMERA_SIZE     0x018A0000
#define MSM_ION_SF_SIZE     MSM_PMEM_SF_SIZE
#define MSM_ION_HEAP_NUM    3
#endif

#define VISION_GPIO_WIFI_IRQ             147
#define VISION_GPIO_WIFI_SHUTDOWN_N       39

#define VISION_GPIO_UART2_RX 	51
#define VISION_GPIO_UART2_TX 	52

#define VISION_GPIO_FLASHLIGHT_TORCH    129
#define VISION_GPIO_TORCH_EN    129
#define VISION_GPIO_FLASHLIGHT_FLASH    128
#define VISION_GPIO_FLASH_EN		128

#define VISION_GPIO_KEYPAD_POWER_KEY		46

#define VISION_GPIO_PROXIMITY_INT_N		18
#define VISION_GPIO_PS_INT_N		18


#define VISION_GPIO_COMPASS_INT		42
#define VISION_GPIO_CHG_INT	180
#define VISION_GPIO_TP_EN			105

#define VISION_GPIO_TP_ATT_N	(20)


#define VISION_LAYOUTS                  { \
                { { 0,  1, 0}, { 1,  0,  0}, {0, 0, -1} }, \
                { { 0, -1, 0}, { 1,  0,  0}, {0, 0, -1} }, \
                { { 1,  0, 0}, { 0, -1,  0}, {0, 0, -1} }, \
                { {-1,  0, 0}, { 0,  0, -1}, {0, 1,  0} }  \
                                        }

#define VISION_EL_EN			(21)
#define VISION_LCD_B0                 (22)
#define VISION_LCD_R0                 (25)
#define VISION_GPIO_PS_HOLD	(29)
#define VISION_LCD_RSTz               (126)
#define VISION_LCD_PCLK               (90)
#define VISION_LCD_DE                 (91)
#define VISION_LCD_VSYNC              (92)
#define VISION_LCD_HSYNC              (93)
#define VISION_LCD_G2                 (94)
#define VISION_LCD_G3                 (95)
#define VISION_LCD_G4                 (96)
#define VISION_LCD_G5                 (97)
#define VISION_LCD_G6                 (98)
#define VISION_LCD_G7                 (99)
#define VISION_LCD_B3                 (100)
#define VISION_LCD_B4                 (101)
#define VISION_LCD_B5                 (102)
#define VISION_LCD_B6                 (103)
#define VISION_LCD_B7                 (104)
#define VISION_LCD_R3                 (105)
#define VISION_LCD_R4                 (106)
#define VISION_LCD_R5                 (107)
#define VISION_LCD_R6                 (108)
#define VISION_LCD_R7                 (109)

/* BT */
#define VISION_GPIO_BT_UART1_RTS      (134)
#define VISION_GPIO_BT_UART1_CTS      (135)
#define VISION_GPIO_BT_UART1_RX       (136)
#define VISION_GPIO_BT_UART1_TX       (137)
#define VISION_GPIO_BT_PCM_OUT        (138)
#define VISION_GPIO_BT_PCM_IN         (139)
#define VISION_GPIO_BT_PCM_SYNC       (140)
#define VISION_GPIO_BT_PCM_CLK        (141)
#define VISION_GPIO_BT_RESET_N        (41)
#define VISION_GPIO_BT_HOST_WAKE      (44)
#define VISION_GPIO_BT_CHIP_WAKE      (50)
#define VISION_GPIO_BT_SHUTDOWN_N     (38)

/* USB */
#define VISION_GPIO_USB_ID1_PIN			(145)
#define VISION_AUDIOz_UART_SW			(95)
#define VISION_USBz_AUDIO_SW				(96)

//#define VISION_SPI_CS2                (87)
//#define VISION_SPI_DO                 (47)
//#define VISION_SPI_DI                 (48)
//#define VISION_SPI_CLK                (45)

#define VISION_GPIO_PS_HOLD	(29)

#define VISION_CAM_PWD                (34)
#define VISION_CAM_A2V85_EN           (37)	/* ANALOG POWER */
#define VISION_CAM_RST                (31)

//#define VISION_SLIDING_INTZ	(18)//Vivo doesn't have sliding keyboard

#define VISION_AUD_MICPATH_SEL         121
#define VISION_AUD_A1026_INT           120
#define VISION_AUD_A1026_WAKEUP        123
#define VISION_AUD_A1026_RESET         122
#define VISION_AUD_A1026_CLK   -1

/* 35mm headset */
#define VISION_OJ_RSTz                (36)

/* EMMC */
#define PMIC_GPIO_INT			(27)

#define VISION_GPIO_EMMC_RST			  (88)

#define VISION_GPIO_UP_RESET_N		(43)
#define VISION_GPIO_UP_INT_N            (142)
/* PMIC GPIO */
#define PMGPIO(x) (x-1)
#define VISION_AUD_SPK_ENO		PMGPIO(18)
#define VISION_AUD_HP_EN		PMGPIO(19)
#define VISION_GPIO_PROXIMITY_EN	PMGPIO(20)
#define VISION_TP_RSTz			PMGPIO(21)
#define VISION_LED_3V3_EN		PMGPIO(22)
#define VISION_VOL_UP			PMGPIO(24)
#define VISION_VOL_DN			PMGPIO(25)
#define VISION_AUD_HP_DETz		PMGPIO(26)
#define VISION_SLIDING_INTz		PMGPIO(33)
#define VISION_CAM_STEP2		PMGPIO(35)
#define VISION_CAM_STEP1		PMGPIO(36)
#define VISION_OJ_ACTION		PMGPIO(37)
#define VISION_SLEEP_CLK2		PMGPIO(39)
#define VISION_DQ_PWRDNz		PMGPIO(40)

/* Camera */
#define CAM1_PWD			(35)
#define CAM1_VCM_PWD	(34)
#define CAM2_PWD			(146)
#define CAM2_RST			(31)
#define CLK_SWITCH		(144)

#endif /* __ARCH_ARM_MACH_MSM_BOARD_VISION_H */
