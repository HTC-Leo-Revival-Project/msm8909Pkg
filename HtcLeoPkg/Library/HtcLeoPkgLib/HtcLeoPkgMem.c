/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
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
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS  16

// DDR attributes
#define DDR_ATTRIBUTES_CACHED               ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED             ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

#define QSD8250_PERIPH_BASE              0xA0000000
#define QSD8250_PERIPH_SIZE              0x0C300000

#define FB_ADDR FixedPcdGet32(PcdMipiFrameBufferAddress)
#define FB_SIZE FixedPcdGet32(PcdMipiFrameBufferWidth)*FixedPcdGet32(PcdMipiFrameBufferHeight)*(FixedPcdGet32(PcdMipiFrameBufferPixelBpp) / 8)

STATIC struct ReservedMemory {
    EFI_PHYSICAL_ADDRESS         Offset;
    EFI_PHYSICAL_ADDRESS         Size;
} ReservedMemoryBuffer [] = {
    { 0x00000000, 0x00100000 },    // APPSBL
    { 0x00100000, 0x00100000 },    // SMEM
    { 0x00200000, 0x00200000 },    // OEMSBL
    { 0x00400000, 0x02100000 },    // AMSS
    { 0x10000000, 0x01800000 },    // QDSP6
    { FB_ADDR,    FB_SIZE    },    // Display Reserved
};

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
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