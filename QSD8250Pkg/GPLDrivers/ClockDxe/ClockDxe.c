/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2007 QUALCOMM Incorporated
 * Copyright (c) 2010 Cotulla
 * Copyright (c) 2012 Shantanu Gupta <shans95g@gmail.com>
 * Copyright (c) 2023 Dominik Kobinski <dominikkobinski314@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/pcom.h>
#include <Library/acpuclock.h>

#if SOC == 1
#include <Chipset/iomap_qsd8250.h>
#elif SOC == 2
#include <Chipset/iomap_msm7230.h>
#endif
#include <Chipset/clock.h>

#include <Protocol/EmbeddedClock.h>

static UINTN ClocksLookup[NR_CLKS];

VOID
FillClocksLookup()
{
  // Fill struct with default values
  for (UINTN i = 0; i < NR_CLKS; i++) {
    ClocksLookup[i] = -1;
  }

  // Fill used clocks
  ClocksLookup[ICODEC_RX_CLK] = 50;
  ClocksLookup[ICODEC_TX_CLK] = 52;
  ClocksLookup[ECODEC_CLK] = 42;
  ClocksLookup[SDAC_MCLK] = 64;
  ClocksLookup[IMEM_CLK] = 55;
  ClocksLookup[GRP_CLK] = 56;
  ClocksLookup[ADM_CLK] = 19;
  ClocksLookup[UART1DM_CLK] = 78;
  ClocksLookup[UART2DM_CLK] = 80;
  ClocksLookup[VFE_AXI_CLK] = 24;
  ClocksLookup[VFE_MDC_CLK] = 40;
  ClocksLookup[VFE_CLK] = 41;
  ClocksLookup[MDC_CLK] = 53;
  ClocksLookup[SPI_CLK] = 95;
  ClocksLookup[MDP_CLK] = 9;
  ClocksLookup[SDC1_CLK] = 66;
  ClocksLookup[SDC2_CLK] = 67;
  ClocksLookup[SDC1_PCLK] = 17;
  ClocksLookup[SDC2_PCLK] = 16;
}


// Cotullaz "new" clock functions
UINTN
CotullaClkSetRate(UINT32 Id, UINTN Rate)
{
	// TODO?
    UINTN Speed = 0;
    switch (Id) {
		case ICODEC_RX_CLK:
			if (Rate > 11289600)     Speed = 9;
			else if (Rate > 8192000) Speed = 8;
			else if (Rate > 6144000) Speed = 7;
			else if (Rate > 5644800) Speed = 6;
			else if (Rate > 4096000) Speed = 5;
			else if (Rate > 3072000) Speed = 4;
			else if (Rate > 2822400) Speed = 3;
			else if (Rate > 2048000) Speed = 2;
			else Speed = 1;
			break;
		case ICODEC_TX_CLK:
			if (Rate > 11289600) 	 Speed = 9;
			else if (Rate > 8192000) Speed = 8;
			else if (Rate > 6144000) Speed = 7;
			else if (Rate > 5644800) Speed = 6;
			else if (Rate > 4096000) Speed = 5;
			else if (Rate > 3072000) Speed = 4;
			else if (Rate > 2822400) Speed = 3;
			else if (Rate > 2048000) Speed = 2;
			else Speed = 1;
			break;
		case SDAC_MCLK:
			if (Rate > 1411200) 	Speed = 9;
			else if (Rate > 1024000)Speed = 8;
			else if (Rate > 768000) Speed = 7;
			else if (Rate > 705600) Speed = 6;
			else if (Rate > 512000) Speed = 5;
			else if (Rate > 384000) Speed = 4;
			else if (Rate > 352800) Speed = 3;
			else if (Rate > 256000) Speed = 2;
			else Speed = 1;
			break;
		case UART1DM_CLK:
			if (Rate > 61440000) 	  Speed = 15;
			else if (Rate > 58982400) Speed = 14;
			else if (Rate > 56000000) Speed = 13;
			else if (Rate > 51200000) Speed = 12;
			else if (Rate > 48000000) Speed = 11;
			else if (Rate > 40000000) Speed = 10;
			else if (Rate > 32000000) Speed = 9;
			else if (Rate > 24000000) Speed = 8;
			else if (Rate > 16000000) Speed = 7;
			else if (Rate > 15360000) Speed = 6;
			else if (Rate > 14745600) Speed = 5;
			else if (Rate >  7680000) Speed = 4;
			else if (Rate >  7372800) Speed = 3;
			else if (Rate >  3840000) Speed = 2;
			else Speed = 1;
			break;
		case UART2DM_CLK:
			if (Rate > 61440000) 	  Speed = 15;
			else if (Rate > 58982400) Speed = 14;
			else if (Rate > 56000000) Speed = 13;
			else if (Rate > 51200000) Speed = 12;
			else if (Rate > 48000000) Speed = 11;
			else if (Rate > 40000000) Speed = 10;
			else if (Rate > 32000000) Speed = 9;
			else if (Rate > 24000000) Speed = 8;
			else if (Rate > 16000000) Speed = 7;
			else if (Rate > 15360000) Speed = 6;
			else if (Rate > 14745600) Speed = 5;
			else if (Rate >  7680000) Speed = 4;
			else if (Rate >  7372800) Speed = 3;
			else if (Rate >  3840000) Speed = 2;
			else Speed = 1;
			break;
		case ECODEC_CLK:
			if (Rate > 2048000) 	Speed = 3;
			else if (Rate > 128000) Speed = 2;
			else Speed = 1;
			break;
		case VFE_MDC_CLK:
			if (Rate == 96000000) 		Speed = 37;
			else if (Rate == 48000000)	Speed = 32;
			else if (Rate == 24000000) 	Speed = 22;
			else if (Rate == 12000000) 	Speed = 14;
			else if (Rate ==  6000000) 	Speed = 6;
			else if (Rate ==  3000000) 	Speed = 1;
			else return 0;
			break;
		case VFE_CLK:
			if (Rate == 36000000) 		Speed = 1;
			else if (Rate == 48000000) 	Speed = 2;
			else if (Rate == 64000000) 	Speed = 3;
			else if (Rate == 78000000) 	Speed = 4;
			else if (Rate == 96000000) 	Speed = 5;
			else return 0;
			break;
		case SPI_CLK:
			if (Rate > 15360000) 		Speed = 5;
			else if (Rate > 9600000) 	Speed = 4;
			else if (Rate > 4800000) 	Speed = 3;
			else if (Rate >  960000) 	Speed = 2;
			else Speed = 1;
			break;
		case SDC1_CLK:
			if (Rate > 50000000) 		Speed = 14;
			else if (Rate > 49152000) 	Speed = 13;
			else if (Rate > 45000000) 	Speed = 12;
			else if (Rate > 40000000) 	Speed = 11;
			else if (Rate > 35000000) 	Speed = 10;
			else if (Rate > 30000000) 	Speed = 9;
			else if (Rate > 25000000) 	Speed = 8;
			else if (Rate > 20000000) 	Speed = 7;
			else if (Rate > 15000000) 	Speed = 6;
			else if (Rate > 10000000) 	Speed = 5;
			else if (Rate > 5000000)  	Speed = 4;
			else if (Rate > 400000)		Speed = 3;
			else if (Rate > 144000)		Speed = 2;
			else Speed = 1;
			break;
		case SDC2_CLK:
			if (Rate > 50000000) 		Speed = 14;
			else if (Rate > 49152000) 	Speed = 13;
			else if (Rate > 45000000) 	Speed = 12;
			else if (Rate > 40000000) 	Speed = 11;
			else if (Rate > 35000000) 	Speed = 10;
			else if (Rate > 30000000) 	Speed = 9;
			else if (Rate > 25000000) 	Speed = 8;
			else if (Rate > 20000000) 	Speed = 7;
			else if (Rate > 15000000) 	Speed = 6;
			else if (Rate > 10000000) 	Speed = 5;
			else if (Rate > 5000000)  	Speed = 4;
			else if (Rate > 400000)		Speed = 3;
			else if (Rate > 144000)		Speed = 2;
			else Speed = 1;
			break;
		case SDC1_PCLK:
		case SDC2_PCLK:
			return 0;
			break;
		default:
			return -1;  
    }
    msm_proc_comm(PCOM_CLK_REGIME_SEC_SEL_SPEED, &ClocksLookup[Id], &Speed);
	
    return 0;
}

/* note: used in lcdc */
EFI_STATUS
ClkSetRate(UINTN Id, UINTN Freq)
{
	UINT32 Retval = 0;
  	Retval = CotullaClkSetRate(Id, Freq);
	if (Retval != -1)
		return Retval;
		
	return -1;
}

UINTN
ClkGetRate(UINT32 Id)
{
	UINTN Rate = 0;

	if (ClocksLookup[Id] != -1) {
		msm_proc_comm(PCOM_CLK_REGIME_SEC_MSM_GET_CLK_FREQ_KHZ, &ClocksLookup[Id], &Rate);
		return Rate;
	}
	else {
		switch(Id) {
		/*case USB_OTG_CLK:
					Rate = get_mdns_host_clock(Id);
					break;*/
		case SDC4_PCLK:
			/* Hardcoded rate */
			Rate = 64000000;
			break;
		default:
			Rate = -1;
		}
	}
	return Rate;
}

UINTN
ClkEnable(UINTN Id)
{
	if (ClocksLookup[Id] != -1) {
    	msm_proc_comm(PCOM_CLK_REGIME_SEC_ENABLE, &ClocksLookup[Id], 0);
    	return ClocksLookup[Id];
  	}
	return -1;
}

VOID
ClkDisable(UINTN Id)
{
	if (ClocksLookup[Id] != -1) {
		msm_proc_comm(PCOM_CLK_REGIME_SEC_DISABLE, &ClocksLookup[Id], 0);
	}
}

int msm_pll_request(unsigned id, unsigned on)
{
	on = !!on;
	return msm_proc_comm(PCOM_CLKCTL_RPC_PLL_REQUEST, &id, &on);
}

/**
 Protocol variable definition
 **/
EMBEDDED_CLOCK_PROTOCOL  gClock = {
  ClkEnable,
  ClkDisable,
  ClkSetRate,
};

EFI_STATUS
EFIAPI
ClockDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS  Status = EFI_SUCCESS;
  	EFI_HANDLE       Handle;
	
	//
	// Make sure the clock protocol has not been installed in the system yet.
	//
	ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEmbeddedClockProtocolGuid);

	FillClocksLookup();

	// Setup Scorpion PLL
	msm_acpu_clock_init(11+6);

	// Install the Embedded Clock Protocol onto a new handle
	Handle = NULL;
	Status = gBS->InstallMultipleProtocolInterfaces (
					&Handle,
					&gEmbeddedClockProtocolGuid,
					&gClock,
					NULL
					);

	if (EFI_ERROR (Status)) {
		Status = EFI_OUT_OF_RESOURCES;
	}

	return EFI_SUCCESS;
}