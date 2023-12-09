/** @file

  Copyright (c) 2011-2017, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/PrePiLib.h>
#include <Library/PrintLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/TimerLib.h>
#include <Library/PerformanceLib.h>
#include <Library/LKEnvLib.h>
#include <Library/MallocLib.h>

#include <Ppi/GuidedSectionExtraction.h>
#include <Ppi/ArmMpCoreInfo.h>
#include <Ppi/SecPerformance.h>

#include "PrePi.h"

UINT64  mSystemMemoryEnd = FixedPcdGet64 (PcdSystemMemoryBase) +
                           FixedPcdGet64 (PcdSystemMemorySize) - 1;

UINTN Width = FixedPcdGet32(PcdMipiFrameBufferWidth);
UINTN Height = FixedPcdGet32(PcdMipiFrameBufferHeight);
UINTN Bpp = FixedPcdGet32(PcdMipiFrameBufferPixelBpp);
UINTN FbAddr = FixedPcdGet32(PcdMipiFrameBufferAddress);

VOID
PaintScreen(
  IN  UINTN   BgColor
)
{
  // Code from FramebufferSerialPortLib
	char* Pixels = (void*)FixedPcdGet32(PcdMipiFrameBufferAddress);

	// Set to black color.
	for (UINTN i = 0; i < Width; i++)
	{
		for (UINTN j = 0; j < Height; j++)
		{
			// Set pixel bit
			for (UINTN p = 0; p < (Bpp / 8); p++)
			{
				*Pixels = (unsigned char)BgColor;
				BgColor = BgColor >> 8;
				Pixels++;
			}
		}
	}
}
/*
VOID
ReconfigFb()
{
  // Paint the FB area to black
  PaintScreen(FB_BGRA8888_BLACK);

  // Move the FB to 0x20000000
  MmioWrite32(MSM_MDP_BASE1 + 0x90008, FbAddr);
  // Stride
  MmioWrite32(MSM_MDP_BASE1 + 0x90004, (Height << 16) | Width);
	MmioWrite32(MSM_MDP_BASE1 + 0x9000c, Width * Bpp / 8);
  // Format (32bpp ARGB)
  MmioWrite32(MDP_DMA_P_CONFIG, DMA_PACK_ALIGN_LSB|DMA_PACK_PATTERN_RGB|DMA_DITHER_EN|
              DMA_OUT_SEL_LCDC|DMA_IBUF_FORMAT_xRGB8888_OR_ARGB8888|DMA_DSTC0G_8BITS|
              DMA_DSTC1B_8BITS|DMA_DSTC2R_8BITS);

  //Ensure all transfers finished
  ArmInstructionSynchronizationBarrier();
  ArmDataMemoryBarrier();
}*/

VOID
ReconfigFb()
{
  //FbAddr = memalign(4096, Width * Height * (Bpp / 8));

  //writel((unsigned) FbAddr, MSM_MDP_BASE1 + 0x90008);
  writel(FbAddr, MSM_MDP_BASE1 + 0x90008);

  writel((Height << 16) | Width, MSM_MDP_BASE1 + 0x90004);
	writel(Width * Bpp / 8, MSM_MDP_BASE1 + 0x9000c);
	//writel(0, MSM_MDP_BASE1 + 0x90010);

  /*writel(DMA_PACK_ALIGN_LSB|DMA_PACK_PATTERN_RGB|DMA_DITHER_EN|DMA_OUT_SEL_LCDC|
	       DMA_IBUF_FORMAT_RGB565|DMA_DSTC0G_8BITS|DMA_DSTC1B_8BITS|DMA_DSTC2R_8BITS,
	       MSM_MDP_BASE1 + 0x90000);*/
  /* LSB|PATTERN|DITHER|DMA_OU_SEL|FORMAT|DMA_DST0G|DST1B|DST2R */
  // Format (32bpp ARGB)
  MmioWrite32(MDP_DMA_P_CONFIG, DMA_PACK_ALIGN_LSB|DMA_PACK_PATTERN_RGB|DMA_DITHER_EN|
              DMA_OUT_SEL_LCDC|DMA_IBUF_FORMAT_xRGB8888_OR_ARGB8888|DMA_DSTC0G_8BITS|
              DMA_DSTC1B_8BITS|DMA_DSTC2R_8BITS);

	int hsync_period  = LCDC_HSYNC_PULSE_WIDTH_DCLK + LCDC_HSYNC_BACK_PORCH_DCLK + Width + LCDC_HSYNC_FRONT_PORCH_DCLK;
	int vsync_period  = (LCDC_VSYNC_PULSE_WIDTH_LINES + LCDC_VSYNC_BACK_PORCH_LINES + Height + LCDC_VSYNC_FRONT_PORCH_LINES) * hsync_period;
	int hsync_start_x = LCDC_HSYNC_PULSE_WIDTH_DCLK + LCDC_HSYNC_BACK_PORCH_DCLK;
	int hsync_end_x   = hsync_period - LCDC_HSYNC_FRONT_PORCH_DCLK - 1;
	int display_hctl  = (hsync_end_x << 16) | hsync_start_x;
	int display_vstart= (LCDC_VSYNC_PULSE_WIDTH_LINES + LCDC_VSYNC_BACK_PORCH_LINES) * hsync_period + LCDC_HSYNC_SKEW_DCLK;
	int display_vend  = vsync_period - (LCDC_VSYNC_FRONT_PORCH_LINES * hsync_period) + LCDC_HSYNC_SKEW_DCLK - 1;
	
  writel((hsync_period << 16) | LCDC_HSYNC_PULSE_WIDTH_DCLK, MSM_MDP_BASE1 + 0xe0004);
	writel(vsync_period, MSM_MDP_BASE1 + 0xe0008);
	writel(LCDC_VSYNC_PULSE_WIDTH_LINES * hsync_period, MSM_MDP_BASE1 + 0xe000c);
	writel(display_hctl, MSM_MDP_BASE1 + 0xe0010);
	writel(display_vstart, MSM_MDP_BASE1 + 0xe0014);
	writel(display_vend, MSM_MDP_BASE1 + 0xe0018);
	writel(0, MSM_MDP_BASE1 + 0xe0028);
	writel(0xff, MSM_MDP_BASE1 + 0xe002c);
	writel(LCDC_HSYNC_SKEW_DCLK, MSM_MDP_BASE1 + 0xe0030);
	writel(0, MSM_MDP_BASE1 + 0xe0038);
	writel(0, MSM_MDP_BASE1 + 0xe001c);
	writel(0, MSM_MDP_BASE1 + 0xe0020);
	writel(0, MSM_MDP_BASE1 + 0xe0024);
	writel(1, MSM_MDP_BASE1 + 0xe0000);
}

VOID
PrePiMain (
  IN  UINTN   UefiMemoryBase,
  IN  UINTN   StacksBase,
  IN  UINT64  StartTimeStamp
  )
{
  EFI_HOB_HANDOFF_INFO_TABLE  *HobList;
  EFI_STATUS                  Status;
  CHAR8                       Buffer[100];
  UINTN                       CharCount;
  UINTN                       StacksSize;
  FIRMWARE_SEC_PERFORMANCE    Performance;

  // Initialize the architecture specific bits
  ArchInitialize ();

  // Reconfigure the framebuffer to 32bpp BGRA8888
  ReconfigFb();

  // There are still a few things to do
  /* enable cp10 and cp11 */
	UINT32 val;
	__asm__ volatile("mrc	p15, 0, %0, c1, c0, 2" : "=r" (val));
	val |= (3<<22)|(3<<20);
	__asm__ volatile("mcr	p15, 0, %0, c1, c0, 2" :: "r" (val));

  ArmInstructionSynchronizationBarrier();
  ArmDataMemoryBarrier();

	/* set enable bit in fpexc */
	__asm__ volatile("mrc  p10, 7, %0, c8, c0, 0" : "=r" (val));
	val |= (1<<30);
	__asm__ volatile("mcr  p10, 7, %0, c8, c0, 0" :: "r" (val));

  /* enable the cycle count register */
	UINT32 en;
	__asm__ volatile("mrc	p15, 0, %0, c9, c12, 0" : "=r" (en));
	en &= ~(1<<3); /* cycle count every cycle */
	en |= 1; /* enable all performance counters */
	__asm__ volatile("mcr	p15, 0, %0, c9, c12, 0" :: "r" (en));

	/* enable cycle counter */
	en = (1<<31);
	__asm__ volatile("mcr	p15, 0, %0, c9, c12, 1" :: "r" (en));

  // Initialize the Serial Port
  SerialPortInitialize ();
  CharCount = AsciiSPrint (
                Buffer,
                sizeof (Buffer),
                "UEFI firmware (version %s built at %a on %a)\n\r",
                (CHAR16 *)PcdGetPtr (PcdFirmwareVersionString),
                __TIME__,
                __DATE__
                );
  SerialPortWrite ((UINT8 *)Buffer, CharCount);

  DEBUG((
        EFI_D_INFO | EFI_D_LOAD,
        "UEFI Memory Base = 0x%p, Stack Base = 0x%p\n",
        UefiMemoryBase,
        StacksBase
    ));

  // Initialize the Debug Agent for Source Level Debugging
  InitializeDebugAgent (DEBUG_AGENT_INIT_POSTMEM_SEC, NULL, NULL);
  SaveAndSetDebugTimerInterrupt (TRUE);

  // Declare the PI/UEFI memory region
  HobList = HobConstructor (
              (VOID *)UefiMemoryBase,
              FixedPcdGet32 (PcdSystemMemoryUefiRegionSize),
              (VOID *)UefiMemoryBase,
              (VOID *)StacksBase // The top of the UEFI Memory is reserved for the stacks
              );
  PrePeiSetHobList (HobList);

  // Initialize MMU and Memory HOBs (Resource Descriptor HOBs)
  Status = MemoryPeim (UefiMemoryBase, FixedPcdGet32 (PcdSystemMemoryUefiRegionSize));
  ASSERT_EFI_ERROR (Status);

  // Create the Stacks HOB (reserve the memory for all stacks)
  if (ArmIsMpCore ()) {
    StacksSize = PcdGet32 (PcdCPUCorePrimaryStackSize) +
                 ((FixedPcdGet32 (PcdCoreCount) - 1) * FixedPcdGet32 (PcdCPUCoreSecondaryStackSize));
  } else {
    StacksSize = PcdGet32 (PcdCPUCorePrimaryStackSize);
  }

  BuildStackHob (StacksBase, StacksSize);

  // TODO: Call CpuPei as a library
  BuildCpuHob (ArmGetPhysicalAddressBits (), PcdGet8 (PcdPrePiCpuIoSize));

  // Store timer value logged at the beginning of firmware image execution
  Performance.ResetEnd = GetTimeInNanoSecond (StartTimeStamp);

  // Build SEC Performance Data Hob
  BuildGuidDataHob (&gEfiFirmwarePerformanceGuid, &Performance, sizeof (Performance));

  // Set the Boot Mode
  SetBootMode (ArmPlatformGetBootMode ());

  // Initialize Platform HOBs (CpuHob and FvHob)
  Status = PlatformPeim ();
  ASSERT_EFI_ERROR (Status);

  // Now, the HOB List has been initialized, we can register performance information
  PERF_START (NULL, "PEI", NULL, StartTimeStamp);

  // SEC phase needs to run library constructors by hand.
  ProcessLibraryConstructorList ();

  // Assume the FV that contains the SEC (our code) also contains a compressed FV.
  Status = DecompressFirstFv ();
  ASSERT_EFI_ERROR (Status);

  // Load the DXE Core and transfer control to it
  Status = LoadDxeCoreFromFv (NULL, 0);
  ASSERT_EFI_ERROR (Status);
}

VOID
CEntryPoint (
  IN  UINTN  MpId,
  IN  UINTN  UefiMemoryBase,
  IN  UINTN  StacksBase
  )
{
  UINT64  StartTimeStamp;

  // Initialize the platform specific controllers
  ArmPlatformInitialize (MpId);

  StartTimeStamp = 0;

  // Data Cache enabled on Primary core when MMU is enabled.
  ArmDisableDataCache ();
  // Invalidate instruction cache
  ArmInvalidateInstructionCache ();
  // Enable Instruction Caches on all cores.
  ArmEnableInstructionCache ();

  // Wait the Primary core has defined the address of the Global Variable region (event: ARM_CPU_EVENT_DEFAULT)
  ArmCallWFE ();

  // Goto primary Main.
  //PrimaryMain (UefiMemoryBase, StacksBase, StartTimeStamp);
  PrePiMain (UefiMemoryBase, StacksBase, StartTimeStamp);

  // DXE Core should always load and never return
  ASSERT (FALSE);
}
