/*
 * Copyright (c) 2007-2010, Code Aurora Forum. All rights reserved.
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
#include <Library/IoLib.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/LKEnvLib.h>
#include <Chipset/reg.h>
#include <Library/pcom.h>
#include <Chipset/clock.h>
#include <Chipset/gpio.h>
#include <Chipset/iomap.h>

#include <Library/pcom_clients.h>

#define PCOM_VREG_SDC PM_VREG_PDOWN_GP6_ID //?
#define PCOM_MPP_FOR_USB_VBUS PM_MPP_16 //?
#define PROC_COMM_END_CMDS 0xFFFF 

void pcom_vreg_control(unsigned vreg, unsigned level, unsigned state)
{
    unsigned s = (state ? PCOM_ENABLE : PCOM_DISABLE);

	/* If turning it ON, set the level first. */
    if(state)
    {
		do
        {
            msm_proc_comm(PCOM_VREG_SET_LEVEL, &vreg, &level);
		}while(PCOM_CMD_SUCCESS != readl(APP_STATUS));
    }
	
	do
    {
        msm_proc_comm(PCOM_VREG_SWITCH, &vreg, &s);
    }while(PCOM_CMD_SUCCESS != readl(APP_STATUS));
}

void pcom_sdcard_power(int state)
{
    unsigned v = PCOM_VREG_SDC;
	unsigned s = (state ? PCOM_ENABLE : PCOM_DISABLE);

	while(1)
	{
		msm_proc_comm(PCOM_VREG_SWITCH, &v, &s);

        if(PCOM_CMD_SUCCESS != readl(APP_STATUS)) {
			DEBUG((EFI_D_INFO, "Error: PCOM_VREG_SWITCH failed...retrying\n"));
		} else {
			DEBUG((EFI_D_INFO, "PCOM_VREG_SWITCH DONE\n"));
			break;
		}
    }
}

void pcom_sdcard_gpio_config(int instance)
{
//Note: GPIO_NO_PULL is for clock lines.
	switch (instance) {
		case 1:
			/* Some cards had crc erorrs on multiblock reads.
			 * Increasing drive strength from 8 to 16 fixed that.
			 */
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(51, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_16MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(52, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_16MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(53, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_16MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(54, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_16MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(55, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_16MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(56, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_16MA), MSM_GPIO_CFG_ENABLE);
			break;

		case 2:
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(62, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(63, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(64, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(65, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(66, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(67, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			break;

		case 3:
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(88, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(89, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(90, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(91, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(92, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(93, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(158, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(159, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(160, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(161, 1, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			break;

		case 4:
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(142, 3, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(143, 3, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(144, 2, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(145, 2, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(146, 3, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			pcom_gpio_tlmm_config(MSM_GPIO_CFG(147, 3, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_PULL_UP, MSM_GPIO_CFG_8MA), MSM_GPIO_CFG_ENABLE);
			break;
    }
}

//---- USB HS related proc_comm clients
void pcom_usb_vbus_power(int state)
{
    unsigned v = PCOM_MPP_FOR_USB_VBUS;
	unsigned s = (PM_MPP__DLOGIC__LVL_VDD << 16) | (state ? PM_MPP__DLOGIC_OUT__CTRL_HIGH : PM_MPP__DLOGIC_OUT__CTRL_LOW);

	msm_proc_comm(PCOM_PM_MPP_CONFIG, &v, &s);

    if(PCOM_CMD_SUCCESS != readl(APP_STATUS)) {
        DEBUG((EFI_D_INFO, "Error: PCOM_MPP_CONFIG failed... not retrying\n"));
    } else {
        DEBUG((EFI_D_INFO, "PCOM_MPP_CONFIG DONE\n"));
    }
}

void pcom_usb_reset_phy(void)
{
	while(1)
	{
        msm_proc_comm(PCOM_MSM_HSUSB_PHY_RESET, 0, 0);

        if(PCOM_CMD_SUCCESS != readl(APP_STATUS)) {
            DEBUG((EFI_D_INFO, "Error: PCOM_MSM_HSUSB_PHY_RESET failed...not retrying\n"));
			break; // remove to retry
        } else {
            DEBUG((EFI_D_INFO, "PCOM_MSM_HSUSB_PHY_RESET DONE\n"));
            break;
        }
    }
}

void pcom_enable_hsusb_clk(void)
{
    pcom_clock_enable(PCOM_USB_HS_CLK);
}

void pcom_disable_hsusb_clk(void)
{
    pcom_clock_disable(PCOM_USB_HS_CLK);
}

//---- SD card related proc_comm clients
void pcom_set_sdcard_clk_flags(int instance, int flags)
{
	switch(instance){
		case 1:
			pcom_set_clock_flags(PCOM_SDC1_CLK, flags);
			break;
		case 2:
			pcom_set_clock_flags(PCOM_SDC2_CLK, flags);
			break;
		case 3:
			pcom_set_clock_flags(PCOM_SDC3_CLK, flags);
			break;
		case 4:
			pcom_set_clock_flags(PCOM_SDC4_CLK, flags);
			break;
	}
}

void pcom_set_sdcard_clk(int instance, int rate)
{
	switch(instance){
		case 1:
			pcom_clock_set_rate(PCOM_SDC1_CLK, rate);
			break;
		case 2:
			pcom_clock_set_rate(PCOM_SDC2_CLK, rate);
			break;
		case 3:
			pcom_clock_set_rate(PCOM_SDC3_CLK, rate);
			break;
		case 4:
			pcom_clock_set_rate(PCOM_SDC4_CLK, rate);
			break;
	}
}

UINT32 pcom_get_sdcard_clk(int instance)
{
	UINT32 rate = 0;
	switch(instance){
		case 1:
			rate = pcom_clock_get_rate(PCOM_SDC1_CLK);
			break;
		case 2:
			rate = pcom_clock_get_rate(PCOM_SDC2_CLK);
			break;
		case 3:
			rate = pcom_clock_get_rate(PCOM_SDC3_CLK);
			break;
		case 4:
			rate = pcom_clock_get_rate(PCOM_SDC4_CLK);
			break;
	}
	return rate;
}

void pcom_enable_sdcard_clk(int instance)
{
	switch(instance){
		case 1:
			pcom_clock_enable(PCOM_SDC1_CLK);
			break;
		case 2:
			pcom_clock_enable(PCOM_SDC2_CLK);
			break;
		case 3:
			pcom_clock_enable(PCOM_SDC3_CLK);
			break;
		case 4:
			pcom_clock_enable(PCOM_SDC4_CLK);
			break;
	}
}

void pcom_disable_sdcard_clk(int instance)
{
	switch(instance){
		case 1:
			pcom_clock_disable(PCOM_SDC1_CLK);
			break;
		case 2:
			pcom_clock_disable(PCOM_SDC2_CLK);
			break;
		case 3:
			pcom_clock_disable(PCOM_SDC3_CLK);
			break;
		case 4:
			pcom_clock_disable(PCOM_SDC4_CLK);
			break;
	}
}

void pcom_enable_sdcard_pclk(int instance)
{
	switch(instance){
		case 1:
			pcom_clock_enable(PCOM_SDC1_PCLK);
			break;
		case 2:
			pcom_clock_enable(PCOM_SDC2_PCLK);
			break;
		case 3:
			pcom_clock_enable(PCOM_SDC3_PCLK);
			break;
		case 4:
			pcom_clock_enable(PCOM_SDC4_PCLK);
			break;
	}
}

void pcom_disable_sdcard_pclk(int instance)
{
	switch(instance){
		case 1:
			pcom_clock_disable(PCOM_SDC1_PCLK);
			break;
		case 2:
			pcom_clock_disable(PCOM_SDC2_PCLK);
			break;
		case 3:
			pcom_clock_disable(PCOM_SDC3_PCLK);
			break;
		case 4:
			pcom_clock_disable(PCOM_SDC4_PCLK);
			break;
	}
}

UINT32 pcom_is_sdcard_clk_enabled(int instance)
{
	UINT32 ret=0;
	switch(instance){
		case 1:
			ret = pcom_clock_is_enabled(PCOM_SDC1_CLK);
			break;
		case 2:
			ret = pcom_clock_is_enabled(PCOM_SDC2_CLK);
			break;
		case 3:
			ret = pcom_clock_is_enabled(PCOM_SDC3_CLK);
			break;
		case 4:
			ret = pcom_clock_is_enabled(PCOM_SDC4_CLK);
			break;
	}
	return ret;
}

UINT32 pcom_is_sdcard_pclk_enabled(int instance)
{
	UINT32 ret=0;
	switch(instance){
		case 1:
			ret = pcom_clock_is_enabled(PCOM_SDC1_PCLK);
			break;
		case 2:
			ret = pcom_clock_is_enabled(PCOM_SDC2_PCLK);
			break;
		case 3:
			ret = pcom_clock_is_enabled(PCOM_SDC3_PCLK);
			break;
		case 4:
			ret = pcom_clock_is_enabled(PCOM_SDC4_PCLK);
			break;
	}
	return ret;
}

//---- UART related proc_comm sdcard clients.
UINT32 pcom_is_uart_clk_enabled(int instance)
{
	return 1;
}

UINT32 pcom_get_uart_clk(int uart_base_addr)
{
	UINT32 rate = 0;
	switch(uart_base_addr){
		case MSM_UART1_BASE:
			rate = pcom_clock_get_rate(PCOM_UART1_CLK);
			break;
		case MSM_UART2_BASE:
			rate = pcom_clock_get_rate(PCOM_UART2_CLK);
			break;
		case MSM_UART3_BASE:
			rate = pcom_clock_get_rate(PCOM_UART3_CLK);
			break;
		default:
			rate = 0;
	}
	return rate;
}

/* LCD_NS_REG, LCD_MD_REG affected */
void pcom_set_lcdc_clk(int rate)
{
	pcom_clock_set_rate(PCOM_MDP_LCDC_PCLK_CLK, rate);
}

void pcom_enable_lcdc_pad_clk(void)
{
	pcom_clock_enable(PCOM_MDP_LCDC_PAD_PCLK_CLK);
}

/* enables LCD_NS_REG->LCD_CLK_BRANCH_ENA */
void pcom_enable_lcdc_clk(void)
{
	pcom_clock_enable(PCOM_MDP_LCDC_PCLK_CLK);
}

/* enables LCD_NS_REG->MNCNTR_EN,LCD_ROOT_ENA, LCD_CLK_EXT_BRANCH_ENA */
UINT32 pcom_get_lcdc_clk(void)
{
    return pcom_clock_get_rate(PCOM_MDP_LCDC_PCLK_CLK);
}

void pcom_end_cmds(void)
{
	msm_proc_comm(PROC_COMM_END_CMDS, 0, 0);
}