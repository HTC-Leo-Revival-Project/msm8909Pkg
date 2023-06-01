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

#include <Ppi/GuidedSectionExtraction.h>
#include <Ppi/ArmMpCoreInfo.h>
#include <Ppi/SecPerformance.h>

#include "PrePi.h"

/* Framebuffer adress and size */
UINT8 *start = (UINT8 *)0x02A00000;
UINT8 *end = (UINT8 *)0x02ABBB00;

UINT64  mSystemMemoryEnd = FixedPcdGet64 (PcdSystemMemoryBase) +
                           FixedPcdGet64 (PcdSystemMemorySize) - 1;

VOID
PaintScreen(int Color)
{
  for (UINT8 *ptr = start; ptr < end; ptr++) {
    *ptr = Color;
  }
}

VOID
InitInterrupts()
{
	MmioWrite32(VIC_INT_CLEAR0, 0xffffffff);
	MmioWrite32(VIC_INT_CLEAR1, 0xffffffff);
	MmioWrite32(VIC_INT_SELECT0, 0);
	MmioWrite32(VIC_INT_SELECT1, 0);
	MmioWrite32(VIC_INT_TYPE0, 0xffffffff);
	MmioWrite32(VIC_INT_TYPE1, 0xffffffff);
	MmioWrite32(VIC_CONFIG, 0);
	MmioWrite32(VIC_INT_MASTEREN, 1);
}

VOID
PrepareForBoot()
{
	// Martijn Stolk's code so kernel will not crash. aux control register
	__asm__ volatile("MRC p15, 0, r0, c1, c0, 1\n"
					 "BIC r0, r0, #0x40\n"
					 "BIC r0, r0, #0x200000\n"
					 "MCR p15, 0, r0, c1, c0, 1");

	// Disable VFP
	//__asm__ volatile("MOV R0, #0\n"
	//				 "FMXR FPEXC, r0");

    // Disable MMU
	__asm__ volatile("MRC p15, 0, r0, c1, c0, 0\n"
					 "BIC r0, r0, #(1<<0)\n"
					 "MCR p15, 0, r0, c1, c0, 0\n"
					 "ISB");
	
	// Invalidate the UTLB
	__asm__ volatile("MOV r0, #0\n"
					 "MCR p15, 0, r0, c8, c7, 0");

	// Clean and invalidate cache - Ensure pipeline flush
	__asm__ volatile("MOV R0, #0\n"
					 "DSB\n"
					 "ISB");

	//__asm__ volatile("BX LR");
}

VOID
CpuSetup()
{
  // do some cpu setup 
	// thanks imbushuo for including this in PrimeG2Pkg 

	// Disable L2 cache
	__asm__ volatile("mrc p15, 0, r0, c1, c0, 1");
	__asm__ volatile("bic r0, r0, #0x00000002");
	__asm__ volatile("mcr p15, 0, r0, c1, c0, 1");

	// Disable Strict alignment checking & Enable Instruction cache
	__asm__ volatile("mrc p15, 0, r0, c1, c0, 0");
	__asm__ volatile("bic r0, r0, #0x00002300");     // clear bits 13, 9:8 (--V- --RS) 
	__asm__ volatile("bic r0, r0, #0x00000005");     // clear bits 0, 2 (---- -C-M) 
	__asm__ volatile("bic r0, r0, #0x00000002");     // Clear bit 1 (Alignment faults) 
	__asm__ volatile("orr r0, r0, #0x00001000");     // set bit 12 (I) enable I-Cache 
	__asm__ volatile("mcr p15, 0, r0, c1, c0, 0");
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

  PaintScreen(Black);

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
  //InitializeDebugAgent (DEBUG_AGENT_INIT_POSTMEM_SEC, NULL, NULL);
  //SaveAndSetDebugTimerInterrupt (TRUE);

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Declare the PI/UEFI memory region\n"));

  // Declare the PI/UEFI memory region
  HobList = HobConstructor (
              (VOID *)UefiMemoryBase,
              FixedPcdGet32 (PcdSystemMemoryUefiRegionSize),
              (VOID *)UefiMemoryBase,
              (VOID *)StacksBase // The top of the UEFI Memory is reserved for the stacks
              );
  PrePeiSetHobList (HobList);

  // Initialize MMU and Memory HOBs (Resource Descriptor HOBs)

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Initialize MMU and Memory HOBs (Resource Descriptor HOBs)\n"));

  Status = MemoryPeim (UefiMemoryBase, FixedPcdGet32 (PcdSystemMemoryUefiRegionSize));
  if (EFI_ERROR(Status))
  {
      DEBUG((EFI_D_ERROR, "Failed to configure MMU\n"));
      CpuDeadLoop();
  }
  else {
     DEBUG((EFI_D_INFO | EFI_D_LOAD, "MMU configured\n"));
  }


  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Create the Stacks HOB (reserve the memory for all stacks)\n"));
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
  //Performance.ResetEnd = GetTimeInNanoSecond (StartTimeStamp);

  // Build SEC Performance Data Hob
  BuildGuidDataHob (&gEfiFirmwarePerformanceGuid, &Performance, sizeof (Performance));

  // Set the Boot Mode
  SetBootMode (ArmPlatformGetBootMode ());

  // Initialize Platform HOBs (CpuHob and FvHob)
  Status = PlatformPeim ();
  if (EFI_ERROR(Status))
  {
      DEBUG((EFI_D_ERROR, "Failed to Initialize Platform HOBS\n"));
  }
  else{
     DEBUG((EFI_D_INFO | EFI_D_LOAD, "Platform HOBS Initialized\n"));
  }

  // Initialize interrupts
  InitInterrupts();
  DEBUG((EFI_D_ERROR, "Interrupts inited!\n"));

  // Initialize timer
  MmioWrite32(DGT_ENABLE, 0);
  DEBUG((EFI_D_ERROR, "Timer inited!\n"));

  // SEC phase needs to run library constructors by hand.
  ProcessLibraryConstructorList ();

  // Assume the FV that contains the SEC (our code) also contains a compressed FV.
  Status = DecompressFirstFv ();
  if (EFI_ERROR(Status))
  {
      DEBUG((EFI_D_ERROR, "FV does not contain a compressed FV\n"));
  }else{
     DEBUG((EFI_D_INFO | EFI_D_LOAD, "FV contains a compressed FV\n"));
  }

  // Load the DXE Core and transfer control to it
  Status = LoadDxeCoreFromFv (NULL, 0);
  if (EFI_ERROR(Status))
  {
      DEBUG((EFI_D_ERROR, "Failed to load DXE Core\n"));
  }else{
     DEBUG((EFI_D_INFO | EFI_D_LOAD, "Loading DXE Core\n"));
  }
}

VOID
CEntryPoint (
  IN  UINTN  MpId,
  IN  UINTN  UefiMemoryBase,
  IN  UINTN  StacksBase
  )
{
  UINT64  StartTimeStamp;

  /* Set vector base */
  ArchEarlyInit();

  /* Linux preparation code from lk */
  PrepareForBoot();

  /* Disable L2 cache and strict alignment checks */
  CpuSetup();

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
