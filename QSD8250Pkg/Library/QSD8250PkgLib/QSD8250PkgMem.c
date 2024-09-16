/** @file
*
*  Copyright (c) 2018, Linaro Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

// The total number of descriptors, including the final "end-of-table" descriptor.
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS 12

// DDR attributes
#define DDR_ATTRIBUTES_CACHED               ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED             ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

#define SOC_REGISTERS_ATTRIBUTES            ARM_MEMORY_REGION_ATTRIBUTE_DEVICE

#define QSD8250_PERIPH_BASE              0xA0000000
#define QSD8250_PERIPH_SIZE              0x0C300000

/**
  Return the Virtual Memory Map of your platform
  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.
  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry
**/
VOID
ArmPlatformGetVirtualMemoryMap (
    IN ARM_MEMORY_REGION_DESCRIPTOR **VirtualMemoryMap
) {
  // You are not expected to call this
	ASSERT(FALSE);

  ARM_MEMORY_REGION_ATTRIBUTES  CacheAttributes;
  UINTN                         Index = 0;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  ASSERT (VirtualMemoryMap != NULL);
  DEBUG ((DEBUG_VERBOSE, "Enter: ArmPlatformGetVirtualMemoryMap\n"));
  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR *) AllocatePages (
      EFI_SIZE_TO_PAGES (sizeof (ARM_MEMORY_REGION_DESCRIPTOR) *
      MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
    return;
  }

  CacheAttributes = DDR_ATTRIBUTES_CACHED;

  // SOC registers region
  VirtualMemoryTable[Index].PhysicalBase   = 0x00000000;
  VirtualMemoryTable[Index].VirtualBase    = 0x00000000;
  VirtualMemoryTable[Index].Length         = 0x11800000;
  VirtualMemoryTable[Index].Attributes     = SOC_REGISTERS_ATTRIBUTES;

  // Rearranged system memory regions
  VirtualMemoryTable[++Index].PhysicalBase = 0x11800000;
  VirtualMemoryTable[Index].VirtualBase    = 0x11800000;
  VirtualMemoryTable[Index].Length         = 0x00008000;
  VirtualMemoryTable[Index].Attributes     = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH;

  // FD
  VirtualMemoryTable[++Index].PhysicalBase = 0x11808000;
  VirtualMemoryTable[Index].VirtualBase    = 0x11808000;
  VirtualMemoryTable[Index].Length         = 0x00200000;
  VirtualMemoryTable[Index].Attributes     = CacheAttributes;

  // Free memory
  VirtualMemoryTable[++Index].PhysicalBase = 0x11A08000;
  VirtualMemoryTable[Index].VirtualBase    = 0x11A08000;
  VirtualMemoryTable[Index].Length         = 0x1E5F8000;
  VirtualMemoryTable[Index].Attributes     = CacheAttributes;

  // VIC region
  VirtualMemoryTable[++Index].PhysicalBase = 0xAC000000;
  VirtualMemoryTable[Index].VirtualBase    = 0xAC000000;
  VirtualMemoryTable[Index].Length         = 0x00100000;
  VirtualMemoryTable[Index].Attributes     = SOC_REGISTERS_ATTRIBUTES;

  // GPT region
  VirtualMemoryTable[++Index].PhysicalBase = 0xAC100000;
  VirtualMemoryTable[Index].VirtualBase    = 0xAC100000;
  VirtualMemoryTable[Index].Length         = 0x00100000;
  VirtualMemoryTable[Index].Attributes     = SOC_REGISTERS_ATTRIBUTES;

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase = 0;
  VirtualMemoryTable[Index].VirtualBase    = 0;
  VirtualMemoryTable[Index].Length         = 0;
  VirtualMemoryTable[Index].Attributes     = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT ((Index + 1) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}