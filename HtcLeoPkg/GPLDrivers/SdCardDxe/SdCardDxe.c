/*
 * Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 * Copytight (c) 2023, Dominik Kobinski <dominikkobinski314@gmail.com>
 *
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
 
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>

#include <Library/gpio.h>
#include <Library/Sdcard.h>
#include <Chipset/iomap.h>
#include <Library/reg.h>
#include <Chipset/gpio.h>

#include <Library/adm.h>
#include <Library/pcom_clients.h>
#include <Protocol/GpioTlmm.h>

// Cached copy of the Hardware Gpio protocol instance
TLMM_GPIO *gGpio = NULL;

struct sd_parms sdcn;
static int high_capacity = 0;
int scr_valid = 0;
UINT32 scr[2];
UINT16 rca;

/* Function prototype */
static void mmc_decode_csd(UINT32 * resp);
static void mmc_decode_cid(UINT32 * resp);

/*
 *  Set SD MCLK speed (NOTE: Using pcom)
 */
static int SD_MCLK_set(enum SD_MCLK_speed speed)
{
    //#ifdef USE_PROC_COMM
	// SDCn_NS_REG clk enable bits are turned on automatically as part of
	// setting clk speed. No need to enable sdcard clk explicitely
    pcom_set_sdcard_clk(sdcn.instance, speed);
	return(1);
}

/*
 * Initialize the specified SD card controller.
 */
static int SDCn_init(UINT32 instance)
{
	// Initialize SD structure based on the controller instance.
	sdcn.instance = instance;
	switch(instance) {
		case 1:
			sdcn.base 				= SDC1_BASE;
			sdcn.ns_addr 			= SDC1_NS_REG;
			sdcn.md_addr 			= SDC1_MD_REG;
			sdcn.row_reset_mask 	= ROW_RESET__SDC1___M;
			sdcn.glbl_clk_ena_mask	= GLBL_CLK_ENA__SDC1_H_CLK_ENA___M;
			sdcn.adm_crci_num 		= ADM_CRCI_SDC1;
			break;
		case 2:
			sdcn.base 				= SDC2_BASE;
			sdcn.ns_addr 			= SDC2_NS_REG;
			sdcn.md_addr 			= SDC2_MD_REG;
			sdcn.row_reset_mask 	= ROW_RESET__SDC2___M;
			sdcn.glbl_clk_ena_mask 	= GLBL_CLK_ENA__SDC2_H_CLK_ENA___M;
			sdcn.adm_crci_num 		= ADM_CRCI_SDC2;
			break;
		case 3:
			sdcn.base 				= SDC3_BASE;
			sdcn.ns_addr 			= SDC3_NS_REG;
			sdcn.md_addr 			= SDC3_MD_REG;
			sdcn.row_reset_mask 	= ROW_RESET__SDC3___M;
			sdcn.glbl_clk_ena_mask 	= GLBL_CLK_ENA__SDC3_H_CLK_ENA___M;
			sdcn.adm_crci_num 		= ADM_CRCI_SDC3;
			break;
		case 4:
			sdcn.base 				= SDC4_BASE;
			sdcn.ns_addr		 	= SDC4_NS_REG;
			sdcn.md_addr 			= SDC4_MD_REG;
			sdcn.row_reset_mask 	= ROW_RESET__SDC4___M;
			sdcn.glbl_clk_ena_mask 	= GLBL_CLK_ENA__SDC4_H_CLK_ENA___M;
			sdcn.adm_crci_num		= ADM_CRCI_SDC4;
			break;
		default:
			return(0); // Error: incorrect instance number
   }

	// Set the appropriate bit in GLBL_CLK_ENA to start the HCLK
	// Save the initial value of the bit for restoring later
	sdcn.glbl_clk_ena_initial = (sdcn.glbl_clk_ena_mask & MmioRead32(GLBL_CLK_ENA));
	//printf("BEFORE:: GLBL_CLK_ENA=0x%08x\n",MmioRead32(GLBL_CLK_ENA));
	//printf("sdcn.glbl_clk_ena_initial = %d\n", sdcn.glbl_clk_ena_initial);
	if (sdcn.glbl_clk_ena_initial == 0)
		MmioWrite32(GLBL_CLK_ENA, MmioRead32(GLBL_CLK_ENA) | sdcn.glbl_clk_ena_mask);

	//printf("AFTER_ENABLE:: GLBL_CLK_ENA=0x%08x\n",MmioRead32(GLBL_CLK_ENA));

	// Set SD MCLK to 400KHz for card detection
	SD_MCLK_set(400000);

    //#ifdef USE_DM
	// Remember the initial value for restore
	sdcn.adm_ch8_rslt_conf_initial = MmioRead32(ADM_REG_CH8_RSLT_CONF);
    //#endif

	// Configure GPIOs using proc_comm
	//sdcard_gpio_config(sdcn.instance); (use if pcom doesn't work)
    pcom_sdcard_gpio_config(instance);

	// Clear all status bits
	MmioWrite32(sdcn.base + MCI_CLEAR, 0x07FFFFFF);

	// Disable all interrupts sources
	MmioWrite32(sdcn.base + MCI_INT_MASK0, 0x0);
	MmioWrite32(sdcn.base + MCI_INT_MASK1, 0x0);

	// Power control to the card, enable MCICLK with power save mode
	// disabled, otherwise the initialization clock cycles will be
	// shut off and the card will not initialize.
	MmioWrite32(sdcn.base + MCI_POWER, MCI_POWER__CONTROL__POWERON);
	MmioWrite32(sdcn.base + MCI_CLK, (MCI_CLK__SELECT_IN__ON_THE_FALLING_EDGE_OF_MCICLOCK << MCI_CLK__SELECT_IN___S) |MCI_CLK__ENABLE___M);

	// Delay
	MicroSecondDelay(1000);

	return(1);
}

static int mmc_send_cmd(UINT16 cmd, UINT32 arg, UINT32 response[])
{
	UINT8 cmd_timeout = 0, cmd_crc_fail = 0, cmd_response_end = 0, n;
	UINT8 cmd_index = cmd & MCI_CMD__CMD_INDEX___M;
	UINT32 mci_status;

	// Program command argument before programming command register
	MmioWrite32(sdcn.base + MCI_ARGUMENT, arg);

	// Program command index and command control bits
	MmioWrite32(sdcn.base + MCI_CMD, cmd);

	// Check response if enabled
	if (cmd & MCI_CMD__RESPONSE___M) {
      // This condition has to be there because CmdCrcFail flag
      // sometimes gets set before CmdRespEnd gets set
      // Wait till one of the CMD flags is set
		while(!(cmd_crc_fail || cmd_timeout || cmd_response_end ||
              ((cmd_index == CMD5 || cmd_index == ACMD41 || cmd_index == CMD1) && cmd_crc_fail)))
		{
			mci_status = MmioRead32(sdcn.base + MCI_STATUS);
			// command crc failed if flag is set
			cmd_crc_fail = (mci_status & MCI_STATUS__CMD_CRC_FAIL___M) >> MCI_STATUS__CMD_CRC_FAIL___S;
			// command response received w/o error
			cmd_response_end = (mci_status & MCI_STATUS__CMD_RESPONSE_END___M) >> MCI_STATUS__CMD_RESPONSE_END___S;
			// if CPSM intr disabled ==> timeout enabled
			if (!(cmd & MCI_CMD__INTERRUPT___M)) {
				// command timed out flag is set
				cmd_timeout = (mci_status & MCI_STATUS__CMD_TIMEOUT___M) >>	MCI_STATUS__CMD_TIMEOUT___S;
			}
		}

		// clear 'CmdRespEnd' status bit
		MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__CMD_RESP_END_CLT___M);

		// Wait till CMD_RESP_END flag is cleared to handle slow 'mclks'
		while ((MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__CMD_RESPONSE_END___M));

		// Clear both just in case
		MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__CMD_TIMOUT_CLR___M);
		MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__CMD_CRC_FAIL_CLR___M);

		// Read the contents of 4 response registers (irrespective of
		// long/short response)
		for(n = 0; n < 4; n++) {
			response[n] = MmioRead32(sdcn.base + MCI_RESPn(n));
		}
		if ((cmd_response_end == 1)
		|| ((cmd_crc_fail == 1) && (cmd_index == CMD5 || cmd_index == ACMD41 || cmd_index == CMD1)))
		{
			return(1);
		} else
		// Assuming argument (or RCA) value of 0x0 will be used for CMD5 and
		// CMD55 before card identification/initialization
		if ((cmd_index == CMD5  && arg == 0 && cmd_timeout == 1)
		|| (cmd_index == CMD55 && arg == 0 && cmd_timeout == 1))
		{
			return(1);
		} else {	
			return(0);
		}
	} else /* No response is required */ {
		// Wait for 'CmdSent' flag to be set in status register
		while(!(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__CMD_SENT___M));

		// clear 'CmdSent' status bit
		MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__CMD_SENT_CLR___M);

		// Wait till CMD_SENT flag is cleared. To handle slow 'mclks'
		while(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__CMD_SENT___M);

		return(1);
	}
}

int card_identification_selection(UINT32 cid[], UINT16* rca, UINT8* num_of_io_func)
{
	UINT32 i;
	UINT16 cmd;
	UINT32 response[4] = {0};
	UINT32 arg;
	UINT32 hc_support = 0;

	DEBUG((EFI_D_ERROR, "CMD0 - put the card in the idle state\n"));
	cmd = CMD0 | MCI_CMD__ENABLE___M;
	if (!mmc_send_cmd(cmd, 0x00000000, response))
		return(0);

	DEBUG((EFI_D_ERROR, "CMD8 - send interface condition\n"));
	// CMD8 - send interface condition
	// Attempt to init SD v2.0 features
	// voltage range 2.7-3.6V, check pattern AA
	arg = 0x000001AA;
	cmd = CMD8 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, arg, response))
	{
		hc_support = 0;          // no response, not high capacity
		DEBUG((EFI_D_ERROR, "Not HC\n"));
	}
	else
	{
		hc_support = (1 << 30);  // HCS bit for ACMD41
		DEBUG((EFI_D_ERROR, "HC\n"));
	}
	NanoSecondDelay(1000);
	
	// CMD55
	// Send before any application specific command
	// Use RCA address = 0 during initialization
	cmd = CMD55 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, 0x00000000, response))
		return(0);

	// ACMD41
	// Reads OCR register contents
	arg = 0x00FF8000 | hc_support;
	cmd = ACMD41 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, arg, response))
		return(0);

	// If stuck in an infinite loop after CMD55 or ACMD41 -
	// the card might have gone into inactive state w/o accepting vdd range
	// sent with ACMD41
	// Loop till power up status (busy) bit is set
	while (!(response[0] & 0x80000000))	{
		// A short delay here after the ACMD41 will prevent the next
		// command from failing with a CRC error when I-cache is enabled.
		NanoSecondDelay(1000);

		cmd = CMD55 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
		if (!mmc_send_cmd(cmd, 0x00000000, response))
			return(0);

		cmd = ACMD41 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
		if (!mmc_send_cmd(cmd, arg, response))
			return(0);
	}

	// Check to see if this is a high capacity SD (SDHC) card.
	if ((response[0] & hc_support) != 0)
		high_capacity = 1;

	// A short delay here after the ACMD41 will prevent the next
	// command from failing with a CRC error when I-cache is enabled.
	NanoSecondDelay(1000);

	// CMD2
	// Reads CID register contents - long response R2
	cmd = CMD2 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M | MCI_CMD__LONGRSP___M;
	if (!mmc_send_cmd(cmd, 0x00000000, response))
		return(0);
	for (i = 0; i < 4; i++)
		cid[i] = response[i];

	// CMD3
	cmd = CMD3 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, 0x00000000, response))
		return(0);
	*rca = response[0] >> 16;

	return(1);
}

//static 
int card_set_block_size(UINT32 size)
{
	UINT16 cmd;
	UINT32 response[4];

	cmd = CMD16 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, size, response))
		return(0);
	else
		return(1);
}

static int read_SCR_register(UINT16 rca)
{
	UINT16 cmd;
	UINT32 response[4] = {0};
	UINT32 mci_status;

	MmioWrite32(sdcn.base + MCI_DATA_TIMER, RD_DATA_TIMEOUT);
	MmioWrite32(sdcn.base + MCI_DATA_LENGTH, 8);  // size of SCR

	// ZZZZ is the following step necessary?

	// Set card block length to 8
	if (!card_set_block_size(8))
		return(0);

	// Write data control register
	MmioWrite32(sdcn.base + MCI_DATA_CTL, MCI_DATA_CTL__ENABLE___M | MCI_DATA_CTL__DIRECTION___M | (8 << MCI_DATA_CTL__BLOCKSIZE___S));

	// CMD55   APP_CMD follows
	cmd = CMD55 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
		return(0);

	// ACMD51  SEND_SCR
	cmd = ACMD51 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, 0, response))
		return(0);

	do {
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);
	} while ((mci_status & MCI_STATUS__RXDATA_AVLBL___M) == 0);

	scr[0] = MmioRead32(sdcn.base + MCI_FIFO);
	scr[0] = Byte_swap32(scr[0]);
	scr[1] = MmioRead32(sdcn.base + MCI_FIFO);
	scr[1] = Byte_swap32(scr[1]);
	scr_valid = 1;

	return(1);
}

static int card_transfer_init(UINT16 rca, UINT32 csd[], UINT32 cid[])
{
	UINT16 cmd, i;
	UINT32 response[4] = {0};

	// CMD9    SEND_CSD
	cmd = CMD9 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M | MCI_CMD__LONGRSP___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
		return(0);
	for (i=0; i<4; i++)
		csd[i] = response[i];

	// CMD10   SEND_CID
	cmd = CMD10 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M | MCI_CMD__LONGRSP___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
		return(0);
	for (i=0; i<4; i++)
		cid[i] = response[i];

	// CMD7    SELECT (RCA!=0)
	cmd = CMD7 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
		return(0);

	// Card is now in Transfer State

	if (!read_SCR_register(rca))
		return(0);

    //4 bit mode
	// CMD55   APP_CMD follows
	cmd = CMD55 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
		return(0);

	// ACMD6   SET_BUS_WIDTH
	cmd = ACMD6 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, 2, response)) // 4 bit bus
		return(0);

	// CMD13   SEND_STATUS
	cmd = CMD13 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
      return(0);

	return(1);
}

static int check_clear_read_status(void)
{
	UINT32 mci_status;
	UINT8  data_block_end;

	// Check status
	do {
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);
		data_block_end = (mci_status & MCI_STATUS__DATA_BLK_END___M) >> MCI_STATUS__DATA_BLK_END___S;
		
		//data_crc_fail
		if ((mci_status & MCI_STATUS__DATA_CRC_FAIL___M) >> MCI_STATUS__DATA_CRC_FAIL___S)
			return(0);
			
		//data_timeout
		if ((mci_status & MCI_STATUS__DATA_TIMEOUT___M) >> MCI_STATUS__DATA_TIMEOUT___S)
			return(0);
			
		//rx_overrun
		if ((mci_status & MCI_STATUS__RX_OVERRUN___M) >> MCI_STATUS__RX_OVERRUN___S)
			return(0);
			
		//start_bit_err
		if ((mci_status & MCI_STATUS__START_BIT_ERR___M) >> MCI_STATUS__START_BIT_ERR___S)
			return(0);
	}while(!data_block_end);

	// Clear
	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_BLK_END_CLR___M);

	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_CRC_FAIL_CLR___M | MCI_CLEAR__DATA_TIMEOUT_CLR___M | MCI_CLEAR__RX_OVERRUN_CLR___M | MCI_CLEAR__START_BIT_ERR_CLR___M);

	while(!(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__DATAEND___M));
	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_END_CLR___M);

	while(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__DATAEND___M);
	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_BLK_END_CLR___M);

	return(1);
}

static int check_clear_write_status(void)
{
	UINT32 mci_status;
	UINT8  data_block_end;

	// Check status
	do {
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);
		data_block_end = (mci_status & MCI_STATUS__DATA_BLK_END___M) >> MCI_STATUS__DATA_BLK_END___S;
		
		//data_crc_fail
		if ((mci_status & MCI_STATUS__DATA_CRC_FAIL___M) >> MCI_STATUS__DATA_CRC_FAIL___S)
			return(0);
			
		//data_timeout
		if ((mci_status & MCI_STATUS__DATA_TIMEOUT___M) >> MCI_STATUS__DATA_TIMEOUT___S)
			return(0);
			
		//tx_underrun
		if ((mci_status & MCI_STATUS__TX_UNDERRUN___M) >> MCI_STATUS__TX_UNDERRUN___S)	
			return(0);
			
		//start_bit_err
		if ((mci_status & MCI_STATUS__START_BIT_ERR___M) >> MCI_STATUS__START_BIT_ERR___S)
			return(0);
	}while(!data_block_end);

	// Clear
	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_BLK_END_CLR___M);

	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_CRC_FAIL_CLR___M | MCI_CLEAR__DATA_TIMEOUT_CLR___M | MCI_CLEAR__TX_UNDERRUN_CLR___M);

	while(!(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__DATAEND___M));
	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_END_CLR___M);

	while(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__DATAEND___M);
	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__DATA_BLK_END_CLR___M);

	return(1);
}

static int read_SD_status(UINT16 rca)
{
	UINT16 cmd;
	UINT32 response[4] = {0};
	UINT32 mci_status;
	UINT32 data[16];
	UINT32 i;

	MmioWrite32(sdcn.base + MCI_DATA_TIMER, RD_DATA_TIMEOUT);
	MmioWrite32(sdcn.base + MCI_DATA_LENGTH, 64);  // size of SD status

	// Set card block length to 64
	if (!card_set_block_size(64))
		return(0);

	// Write data control register
	MmioWrite32(sdcn.base + MCI_DATA_CTL, MCI_DATA_CTL__ENABLE___M | MCI_DATA_CTL__DIRECTION___M | (64 << MCI_DATA_CTL__BLOCKSIZE___S));

	// CMD55   APP_CMD follows
	cmd = CMD55 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
		return(0);

	// ACMD13  SD_STATUS
	cmd = ACMD13 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, 0, response))
		return(0);

	i = 0;
	while(i < 16) {
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);

		if ((mci_status & MCI_STATUS__RXDATA_AVLBL___M) != 0) {
			data[i] =MmioRead32(sdcn.base + MCI_FIFO);
			i++;
		} else
		if ((mci_status & MCI_STATUS__RXACTIVE___M) == 0) {
			// Unexpected status on SD status read.
			return(0);
		}
	}

	if (!check_clear_read_status()) {
		return(0);
	}

	// Byte swap the status dwords
	for (i=0; i<16; i++) {
		data[i] = Byte_swap32(data[i]);
	}

	return(1);
}

static int 
switch_mode(UINT16 rca)
{
#define SWITCH_CHECK  (0 << 31)
#define SWITCH_SET    (1 << 31)
	UINT16 cmd;
	UINT32 response[4] = {0};
	UINT32 arg;
	UINT32 data[16];     // 512 bits
	UINT32 i;
	UINT32 mci_status;

	// Only cards that comply with SD Physical Layer Spec Version 1.1 or greater support
	// CMD6. Need to check the spec level in the SCR register before sending CMD6.
	if (scr_valid != 1)
		return(0);

	if (((scr[0] & 0x0F000000) >> 24) == 0)
		return(0);

	MmioWrite32(sdcn.base + MCI_DATA_TIMER, RD_DATA_TIMEOUT);
	MmioWrite32(sdcn.base + MCI_DATA_LENGTH, 64);  // size of switch data

	// Set card block length to 64 bytes
	if (!card_set_block_size(64))
		return(0);

	// Write data control register
	MmioWrite32(sdcn.base + MCI_DATA_CTL, MCI_DATA_CTL__ENABLE___M | MCI_DATA_CTL__DIRECTION___M | (64 << MCI_DATA_CTL__BLOCKSIZE___S));

	// CMD6 Check Function
	cmd = CMD6 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	arg = SWITCH_CHECK | 0x00FFFFF1;  // check if High-Speed is supported
	if (!mmc_send_cmd(cmd, arg, response))
		return(0);

	i = 0;
	while(i < 16) {
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);

		if ((mci_status & MCI_STATUS__RXDATA_AVLBL___M) != 0) {
			data[i] = MmioRead32(sdcn.base + MCI_FIFO);
			i++;
		} else
		if ((mci_status & MCI_STATUS__RXACTIVE___M) == 0) {
			// Unexpected status on SD status read.
			return(0);
		}
	}

	if (!check_clear_read_status())
		return(0);

	// Byte swap the status.
	for (i=0; i<16; i++) {
		data[i] = Byte_swap32(data[i]);
	}

	// Check to see if High-Speed mode is supported.
	// Look at bit 1 of the Function Group 1 information field.
	// This is bit 401 of the 512 byte switch status.
	if ((data[3] & 0x00020000) == 0)
		return(0);

	// Check to see if we can switch to function 1 in group 1.
	// This is in bits 379:376 of the 512 byte switch status.
	if ((data[4] & 0x0F000000) != 0x01000000)
		return(0);

	// At this point it is safe to change to high-speed mode.

	// Write data control register
	MmioWrite32(sdcn.base + MCI_DATA_CTL, MCI_DATA_CTL__ENABLE___M | MCI_DATA_CTL__DIRECTION___M | (64 << MCI_DATA_CTL__BLOCKSIZE___S));

	// CMD6 Set Function
	cmd = CMD6 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	arg = SWITCH_SET | 0x00FFFFF1;  // Set function 1 to High-Speed
	if (!mmc_send_cmd(cmd, arg, response))
		return(0);

	i = 0;
	while(i < 16) {
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);

		if ((mci_status & MCI_STATUS__RXDATA_AVLBL___M) != 0) {
			data[i] = MmioRead32(sdcn.base + MCI_FIFO);
			i++;
		} else
		if ((mci_status & MCI_STATUS__RXACTIVE___M) == 0) {
			// Unexpected status on SD status read.
			return(0);
		}
	}

	if (!check_clear_read_status())
		return(0);

	// Byte swap the status.
	for (i=0; i<16; i++) {
		data[i] = Byte_swap32(data[i]);
	}

	// Check to see if there was a successful switch to high speed mode.
	// This is in bits 379:376 of the 512 byte switch status.
	if ((data[4] & 0x0F000000) != 0x01000000)
		return(0);

	return(1);
}

UINTN
SdCardInit()
{
    UINT32 cid[4] = {0};
    UINT32 csd[4] = {0};
    UINT8  dummy;
    UINT32 buffer[128];
    UINT32 temp32;

    // SD Init
    if (!SDCn_init(SDC_INSTANCE)) {
		DEBUG((EFI_D_ERROR,"SD - error initializing (SDCn_init)\n"));
		
		return EFI_D_ERROR;
    }
	DEBUG((EFI_D_ERROR,"Controller inited\n"));

    // Run card ID sequence
    if (!card_identification_selection(cid, &rca, &dummy)) {
		DEBUG((EFI_D_ERROR,"SD - error initializing (card_identification_selection)\n"));
       
		return EFI_D_ERROR;
    }

    // Change SD clock configuration, set PWRSAVE and FLOW_ENA
    MmioWrite32(sdcn.base + MCI_CLK, MmioRead32(sdcn.base + MCI_CLK) |	MCI_CLK__PWRSAVE___M | MCI_CLK__FLOW_ENA___M);

    if (!card_transfer_init(rca, csd, cid)) {
		DEBUG((EFI_D_ERROR,"SD - error initializing (card_transfer_init)\n"));
		return EFI_D_ERROR;
    }

    // Card is now in four bit mode, do the same with the clock
    temp32 = MmioRead32(sdcn.base + MCI_CLK);
    temp32 &= ~MCI_CLK__WIDEBUS___M;
    MmioWrite32(sdcn.base + MCI_CLK, temp32 | (MCI_CLK__WIDEBUS__4_BIT_MODE << MCI_CLK__WIDEBUS___S));

    if (!read_SD_status(rca)) {
		DEBUG((EFI_D_ERROR,"SD - error reading SD status\n\r"));
		return EFI_D_ERROR;
    }
	
    // The card is now in data transfer mode, standby state.
    // Increase MCLK to 25MHz
    SD_MCLK_set(MCLK_25MHz);

    // Put card in high-speed mode and increase the SD MCLK if supported.
    if (switch_mode(rca) == 1) {
       NanoSecondDelay(1000);
       // Increase MCLK to 48MHz
       SD_MCLK_set(MCLK_48MHz);
       /* Card is in high speed mode, use feedback clock. */
       temp32 = MmioRead32(sdcn.base + MCI_CLK);
       temp32 &= ~(MCI_CLK__SELECT_IN___M);
       temp32 |= (MCI_CLK__SELECT_IN__USING_FEEDBACK_CLOCK << MCI_CLK__SELECT_IN___S);
       MmioWrite32(sdcn.base + MCI_CLK, temp32);
       NanoSecondDelay(1000);
    }

    if (!card_set_block_size(BLOCK_SIZE)) {
		DEBUG((EFI_D_ERROR, "SD - Error setting block size\n\r"));
		return EFI_D_ERROR;
    }

    // Valid SD card found
    mmc_decode_csd(csd);
    mmc_decode_cid(cid);
	return 0;
}

static int read_a_block(UINT32 block_number, UINT32 read_buffer[])
{
	UINT16 cmd, byte_count;
	UINT32 mci_status, response[4];
	UINT32 address;

	if (high_capacity == 0)
		address = block_number * BLOCK_SIZE;
	else
		address = block_number;

	// Set timeout and data length
	MmioWrite32(sdcn.base + MCI_DATA_TIMER, RD_DATA_TIMEOUT);
	MmioWrite32(sdcn.base + MCI_DATA_LENGTH, BLOCK_SIZE);

	// Write data control register
	MmioWrite32(sdcn.base + MCI_DATA_CTL, MCI_DATA_CTL__ENABLE___M | MCI_DATA_CTL__DIRECTION___M | (BLOCK_SIZE << MCI_DATA_CTL__BLOCKSIZE___S));

	// Send READ_SINGLE_BLOCK command
	cmd = CMD17 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, address, response))
		return(0);

	// Read the block
	byte_count = 0;
	while (byte_count < BLOCK_SIZE)	{
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);
		if ((mci_status & MCI_STATUS__RXDATA_AVLBL___M) != 0) {
			*read_buffer = MmioRead32(sdcn.base + MCI_FIFO);
			read_buffer++;
			byte_count += 4;
		} else
		if ((mci_status & MCI_STATUS__RXACTIVE___M) == 0) {
			//Unexpected status on data read.
			return(0);
		}
	}

	if (!check_clear_read_status())
		return(0);

	return(1);
}

static int write_a_block(UINT32 block_number, UINT32 write_buffer[], UINT16 rca)
{
	UINT16 cmd, byte_count;
	UINT32 mci_status, response[4];
	UINT32 address;

	if (high_capacity == 0)
		address = block_number * BLOCK_SIZE;
	else
		address = block_number;

	// Set timeout and data length
	MmioWrite32(sdcn.base + MCI_DATA_TIMER, WR_DATA_TIMEOUT);
	MmioWrite32(sdcn.base + MCI_DATA_LENGTH, BLOCK_SIZE);

	// Send WRITE_BLOCK command
	cmd = CMD24 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M;
	if (!mmc_send_cmd(cmd, address, response))
		return(0);

	// Write data control register
	MmioWrite32(sdcn.base + MCI_DATA_CTL, MCI_DATA_CTL__ENABLE___M | (BLOCK_SIZE << MCI_DATA_CTL__BLOCKSIZE___S));

	// Write the block
	byte_count = 0;
	while (byte_count < BLOCK_SIZE)	{
		mci_status = MmioRead32(sdcn.base + MCI_STATUS);
		if ((mci_status & MCI_STATUS__TXFIFO_FULL___M) == 0) {
			MmioWrite32(sdcn.base + MCI_FIFO, *write_buffer);
			write_buffer++;
			byte_count += 4;
		}

		if (mci_status &
		   (MCI_STATUS__CMD_CRC_FAIL___M | MCI_STATUS__DATA_TIMEOUT___M | MCI_STATUS__TX_UNDERRUN___M))
			return(0);
	}

	if (!check_clear_write_status())
		return(0);

	// Send SEND_STATUS command (with PROG_ENA, can poll on PROG_DONE below)
	cmd = CMD13 | MCI_CMD__ENABLE___M | MCI_CMD__RESPONSE___M | MCI_CMD__PROG_ENA___M;
	if (!mmc_send_cmd(cmd, (rca << 16), response))
		return(0);

	// Wait for PROG_DONE
	while(!(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__PROG_DONE___M));

	// Clear PROG_DONE and wait until cleared
	MmioWrite32(sdcn.base + MCI_CLEAR, MCI_CLEAR__PROG_DONE_CLR___M);
	while(MmioRead32(sdcn.base + MCI_STATUS) & MCI_STATUS__PROG_DONE___M);

	return(1);
}

UINTN
mmc_bread(UINT32 start, UINT32 blkcnt, void *dst)
{
	int err;
	UINT32 cur, blocks_todo = blkcnt;

	do {
		cur = 1;
		if (!read_a_block(start, dst))
		{
			DEBUG((EFI_D_ERROR, "%s: Failed to read blocks\n", __func__));
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		dst += cur * 512;
	} 
	while (blocks_todo > 0);

	return blkcnt;
}

UINTN
mmc_bwrite(UINT32 start, UINT32 blkcnt, void *dst)
{
	int err;
	UINT32 cur, blocks_todo = blkcnt;

	do {
		cur = 1;
		if (!write_a_block(start, dst, rca))
		{
			DEBUG((EFI_D_ERROR, "%s: Failed to read blocks\n", __func__));
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		dst += cur * 512;
	} 
	while (blocks_todo > 0);

	return blkcnt;
}

/* lk func end */

EFI_BLOCK_IO_MEDIA gMMCHSMedia = 
{
	SIGNATURE_32('s', 'd', 'c', 'c'),         // MediaId
	TRUE,                                    // RemovableMedia
	TRUE,                                     // MediaPresent
	FALSE,                                    // LogicalPartition
	FALSE,                                    // ReadOnly
	FALSE,                                    // WriteCaching
	512,                                      // BlockSize
	4,                                        // IoAlign
	0,                                        // Pad
	0                                         // LastBlock
};

typedef struct
{
	VENDOR_DEVICE_PATH  Mmc;
	EFI_DEVICE_PATH     End;
} MMCHS_DEVICE_PATH;

MMCHS_DEVICE_PATH gMmcHsDevicePath =
{
	{
		HARDWARE_DEVICE_PATH,
		HW_VENDOR_DP,
		(UINT8)(sizeof(VENDOR_DEVICE_PATH)),
		(UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8),
		0xb615f1f5, 0x5088, 0x43cd, 0x80, 0x9c, 0xa1, 0x6e, 0x52, 0x48, 0x7d, 0x00
	},
	{
		END_DEVICE_PATH_TYPE,
		END_ENTIRE_DEVICE_PATH_SUBTYPE,
		{ 
			sizeof(EFI_DEVICE_PATH_PROTOCOL), 
			0 
		}
	}
};

/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
static void mmc_decode_csd(UINT32 * resp)
{
	unsigned int mult, csd_struct;
    unsigned int high_capacity = 0;
    unsigned long int SizeInMB;

	csd_struct = UNSTUFF_BITS(resp, 126, 2);
    switch (csd_struct) {
		case 0:
			break;
		case 1:
			high_capacity = 1;
			break;
		default:
			DEBUG((EFI_D_ERROR, "SD: unrecognised CSD structure version %d\n", csd_struct));
			return;
    }
    gMMCHSMedia.BlockSize = 1 << UNSTUFF_BITS(resp, 80, 4);

    if (high_capacity == 0) {
        mult = 1 << (UNSTUFF_BITS(resp, 47, 3) + 2);
        gMMCHSMedia.LastBlock = (1 + UNSTUFF_BITS(resp, 62, 12)) * mult;
        SizeInMB = gMMCHSMedia.LastBlock * gMMCHSMedia.BlockSize / (1024 * 1024);
    } else {
        // High Capacity SD CSD Version 2.0
        gMMCHSMedia.LastBlock = (1 + UNSTUFF_BITS(resp, 48, 16)) * 1024;
        SizeInMB = ((1 + UNSTUFF_BITS(resp, 48, 16)) * gMMCHSMedia.BlockSize) / 1024;
    }

	DEBUG((EFI_D_ERROR, "Detected: %lu blocks of %lu bytes (%luMB) ",
			gMMCHSMedia.LastBlock,
			gMMCHSMedia.BlockSize,
			SizeInMB));
}

/*
 * Given the decoded CSD structure, decode the raw CID to our CID structure.
 */
static void mmc_decode_cid(UINT32 * resp)
{
	/*
	 * SD doesn't currently have a version field so we will
	 * have to assume we can parse this.
	 */
	DEBUG((EFI_D_ERROR, "Man %02x OEM %c%c \"%c%c%c%c%c\" Date %02u/%04u\n",
			UNSTUFF_BITS(resp, 120, 8), UNSTUFF_BITS(resp, 112, 8),
			UNSTUFF_BITS(resp, 104, 8), UNSTUFF_BITS(resp, 96, 8),
			UNSTUFF_BITS(resp, 88, 8), UNSTUFF_BITS(resp, 80, 8),
			UNSTUFF_BITS(resp, 72, 8), UNSTUFF_BITS(resp, 64, 8),
			UNSTUFF_BITS(resp, 8, 4), UNSTUFF_BITS(resp, 12, 8) + 2000));
    DEBUG((EFI_D_ERROR, "%d.%d\n", UNSTUFF_BITS(resp, 60, 4), UNSTUFF_BITS(resp, 56, 4) ));
	DEBUG((EFI_D_ERROR, "%u\n", UNSTUFF_BITS(resp, 24, 32) ));
}

/**

  Reset the Block Device.

  @param  This                 Indicates a pointer to the calling context.
  @param  ExtendedVerification Driver may perform diagnostics on reset.
  @retval EFI_SUCCESS          The device was reset.
  @retval EFI_DEVICE_ERROR     The device is not functioning properly and could
  not be reset.
  **/
EFI_STATUS
EFIAPI
MMCHSReset(
	IN EFI_BLOCK_IO_PROTOCOL          *This,
	IN BOOLEAN                        ExtendedVerification
)
{
	
	return EFI_SUCCESS;
}

/*
 * Function: MmcReadInternal
 * Arg     : Data address on card, o/p buffer & data length
 * Return  : 0 on Success, non zero on failure
 * Flow    : Read data from the card to out
 */
STATIC UINT32 MmcReadInternal
(
    UINT32 DataAddr, 
    UINT32 *Buf, 
    UINT32 DataLen
)
{
    UINT32 Ret = 0;
    UINT32 BlockSize = gMMCHSMedia.BlockSize;
    UINT32 ReadSize;
    UINT8 *Sptr = (UINT8 *) Buf;

    ASSERT(!(DataAddr % BlockSize));
    ASSERT(!(DataLen % BlockSize));

    WriteBackInvalidateDataCacheRange(Buf, DataLen);

    // Set size 
    ReadSize = BlockSize;

    while (DataLen > ReadSize) 
    {
        Ret = mmc_bread((DataAddr / BlockSize), (ReadSize / BlockSize), (VOID *) Sptr);
        if (Ret == 0)
        {
            DEBUG((EFI_D_ERROR, "Failed Reading block @ %x\n",(UINTN) (DataAddr / BlockSize)));
            return 0;
        }
        Sptr += ReadSize;
        DataAddr += ReadSize;
        DataLen -= ReadSize;
    }

    if (DataLen)
    {
        Ret = mmc_bread((DataAddr / BlockSize), (DataLen / BlockSize), (VOID *) Sptr);
        if (Ret == 0)
        {
            DEBUG((EFI_D_ERROR, "Failed Reading block @ %x\n",(UINTN) (DataAddr / BlockSize)));
            return 1;
        }
    }

    return 1;
}


/**

  Read BufferSize bytes from Lba into Buffer.
  @param  This       Indicates a pointer to the calling context.
  @param  MediaId    Id of the media, changes every time the media is replaced.
  @param  Lba        The starting Logical Block Address to read from
  @param  BufferSize Size of Buffer, must be a multiple of device block size.
  @param  Buffer     A pointer to the destination buffer for the data. The caller is
  responsible for either having implicit or explicit ownership of the buffer.


  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the read.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,

  or the buffer is not on proper alignment.

  EFI_STATUS

  **/
EFI_STATUS
EFIAPI
MMCHSReadBlocks(
	IN EFI_BLOCK_IO_PROTOCOL          *This,
	IN UINT32                         MediaId,
	IN EFI_LBA                        Lba,
	IN UINTN                          BufferSize,
	OUT VOID                          *Buffer
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	UINTN      ret;

	if (BufferSize % gMMCHSMedia.BlockSize != 0) 
    {
		DEBUG((EFI_D_ERROR, "MMCHSReadBlocks: BAD buffer!!!\n"));
    	MicroSecondDelay(5000);
        return EFI_BAD_BUFFER_SIZE;
    }

	if (Buffer == NULL) 
    {
		DEBUG((EFI_D_ERROR, "MMCHSReadBlocks: Invalid parameter!!!\n"));
    	MicroSecondDelay(5000);
        return EFI_INVALID_PARAMETER;
    }

	if (BufferSize == 0) 
    {
		DEBUG((EFI_D_ERROR, "MMCHSReadBlocks: BufferSize = 0\n"));
    	MicroSecondDelay(5000);
        return EFI_SUCCESS;
    }

	ret = MmcReadInternal((UINT64) Lba * 512, Buffer, BufferSize);
	
	if (ret == 1)
    {
        return EFI_SUCCESS;
    }
    else
    {
        DEBUG((EFI_D_ERROR, "MMCHSReadBlocks: Read error!\n"));
        MicroSecondDelay(5000);
        return EFI_DEVICE_ERROR;
    }
    
	return Status;
}

/*
 * Function: MmcWriteInternal
 * Arg     : Data address on card, o/p buffer & data length
 * Return  : 0 on Success, non zero on failure
 * Flow    : Write data from out to the card
 */
STATIC UINT32 MmcWriteInternal
(
    UINT32 DataAddr, 
    UINT32 *Buf, 
    UINT32 DataLen
)
{
    UINT32 Ret = 0;
    UINT32 BlockSize = gMMCHSMedia.BlockSize;
    UINT32 ReadSize;
    UINT8 *Sptr = (UINT8 *) Buf;

    ASSERT(!(DataAddr % BlockSize));
    ASSERT(!(DataLen % BlockSize));

    WriteBackInvalidateDataCacheRange(Buf, DataLen);

    // Set size 
    ReadSize = BlockSize;

    while (DataLen > ReadSize) 
    {
        Ret = mmc_bwrite((DataAddr / BlockSize), (ReadSize / BlockSize), (VOID *) Sptr);
        if (Ret == 0)
        {
            DEBUG((EFI_D_ERROR, "Failed Reading block @ %x\n",(UINTN) (DataAddr / BlockSize)));
            return 0;
        }
        Sptr += ReadSize;
        DataAddr += ReadSize;
        DataLen -= ReadSize;
    }

    if (DataLen)
    {
        Ret = mmc_bwrite((DataAddr / BlockSize), (DataLen / BlockSize), (VOID *) Sptr);
        if (Ret == 0)
        {
            DEBUG((EFI_D_ERROR, "Failed writing block @ %x\n",(UINTN) (DataAddr / BlockSize)));
            return 1;
        }
    }

    return 1;
}

/**

  Write BufferSize bytes from Lba into Buffer.
  @param  This       Indicates a pointer to the calling context.
  @param  MediaId    The media ID that the write request is for.
  @param  Lba        The starting logical block address to be written. The caller is
  responsible for writing to only legitimate locations.
  @param  BufferSize Size of Buffer, must be a multiple of device block size.
  @param  Buffer     A pointer to the source buffer for the data.

  @retval EFI_SUCCESS           The data was written correctly to the device.
  @retval EFI_WRITE_PROTECTED   The device can not be written to.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the write.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
  or the buffer is not on proper alignment.

  **/
EFI_STATUS
EFIAPI
MMCHSWriteBlocks(
	IN EFI_BLOCK_IO_PROTOCOL          *This,
	IN UINT32                         MediaId,
	IN EFI_LBA                        Lba,
	IN UINTN                          BufferSize,
	IN VOID                           *Buffer
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	UINTN      ret;

	if (BufferSize % gMMCHSMedia.BlockSize != 0) 
    {
		DEBUG((EFI_D_ERROR, "MMCHSWriteBlocks: BAD buffer!!!\n"));
    	MicroSecondDelay(5000);
        return EFI_BAD_BUFFER_SIZE;
    }

	if (Buffer == NULL) 
    {
		DEBUG((EFI_D_ERROR, "MMCHSWriteBlocks: Invalid parameter!!!\n"));
    	MicroSecondDelay(5000);
        return EFI_INVALID_PARAMETER;
    }

	if (BufferSize == 0) 
    {
		DEBUG((EFI_D_ERROR, "MMCHSWriteBlocks: BufferSize = 0\n"));
    	MicroSecondDelay(5000);
        return EFI_SUCCESS;
    }

	ret = MmcWriteInternal((UINT64) Lba * 512, Buffer, BufferSize);
	
	if (ret == 1)
    {
        return EFI_SUCCESS;
    }
    else
    {
        DEBUG((EFI_D_ERROR, "MMCHSWriteBlocks: Write error!\n"));
        MicroSecondDelay(5000);
        return EFI_DEVICE_ERROR;
    }
	
	return Status;
}


/**

  Flush the Block Device.
  @param  This              Indicates a pointer to the calling context.
  @retval EFI_SUCCESS       All outstanding data was written to the device
  @retval EFI_DEVICE_ERROR  The device reported an error while writting back the data
  @retval EFI_NO_MEDIA      There is no media in the device.

  **/
EFI_STATUS
EFIAPI
MMCHSFlushBlocks(
	IN EFI_BLOCK_IO_PROTOCOL  *This
)
{
	return EFI_SUCCESS;
}


EFI_BLOCK_IO_PROTOCOL gBlockIo = {
	EFI_BLOCK_IO_INTERFACE_REVISION,   // Revision
	&gMMCHSMedia,                      // *Media
	MMCHSReset,                        // Reset
	MMCHSReadBlocks,                   // ReadBlocks
	MMCHSWriteBlocks,                  // WriteBlocks
	MMCHSFlushBlocks                   // FlushBlocks
};

EFI_STATUS
EFIAPI
SdCardInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS  Status;
    UINTN       IsSdPresent = 0;

    // Find the gpio controller protocol.  ASSERT if not found.
    Status = gBS->LocateProtocol (&gTlmmGpioProtocolGuid, NULL, (VOID **)&gGpio);
    ASSERT_EFI_ERROR (Status);

    if (!gGpio->Get(HTCLEO_GPIO_SD_STATUS))
    {
        // Enable SD
        DEBUG((EFI_D_ERROR, "SD Card inserted!\n"));
        SdCardInit();

        UINT8 BlkDump[512];
		ZeroMem(BlkDump, 512);
		BOOLEAN FoundMbr = FALSE;

		for (UINTN i = 0; i <= MIN(gMMCHSMedia.LastBlock, 50); i++)
		{
            int blk = mmc_bread(i, 1, &BlkDump);
            if (blk)
            {
                if (BlkDump[510] == 0x55 && BlkDump[511] == 0xAA)
                {
                    DEBUG((EFI_D_INFO, "MBR found at %d \n", i));
                    FoundMbr = TRUE;
                    break;
                }
                DEBUG((EFI_D_INFO, "MBR not found at %d \n", i));
            }
		}    
        if (!FoundMbr)
        {
            DEBUG((EFI_D_ERROR, "(Protective) MBR not found \n"));
            CpuDeadLoop();
        }

		//Publish BlockIO.
		Status = gBS->InstallMultipleProtocolInterfaces(
			&ImageHandle,
			&gEfiBlockIoProtocolGuid, &gBlockIo,
			&gEfiDevicePathProtocolGuid, &gMmcHsDevicePath,
			NULL
			);
    }
    else {
        // TODO: Defer installing protocol until card is found
        DEBUG((EFI_D_ERROR, "SD Card NOT inserted!\n"));
        return EFI_DEVICE_ERROR;
    }

	return Status;
}