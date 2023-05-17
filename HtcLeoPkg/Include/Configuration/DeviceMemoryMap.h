#ifndef _DEVICE_MEMORY_MAP_H_
#define _DEVICE_MEMORY_MAP_H_

#include <Library/ArmLib.h>

#define MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT 32

/* Below flag is used for system memory */
#define SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES \
                EFI_RESOURCE_ATTRIBUTE_PRESENT |                 \
                EFI_RESOURCE_ATTRIBUTE_INITIALIZED |             \
                EFI_RESOURCE_ATTRIBUTE_TESTED |                  \
                EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |             \
                EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |       \
                EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
                EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |    \
                EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE

typedef enum {
	NoHob,
	AddMem,
	AddDev,
	MaxMem
} DeviceMemoryAddHob;

typedef struct {
	EFI_PHYSICAL_ADDRESS         Address;
	UINT64                       Length;
	EFI_RESOURCE_TYPE            ResourceType;
	EFI_RESOURCE_ATTRIBUTE_TYPE	 ResourceAttribute;
	ARM_MEMORY_REGION_ATTRIBUTES ArmAttributes;
	DeviceMemoryAddHob					 HobOption;
	EFI_MEMORY_TYPE              MemoryType;
} ARM_MEMORY_REGION_DESCRIPTOR_EX, *PARM_MEMORY_REGION_DESCRIPTOR_EX;

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] =
{
  /* Address, Length, ResourceType, Resource Attribute, ARM MMU Attribute, HobOption, EFI Memory Type */

  /* HLOS 0 */
  { 0x00000000, 0x00100000, EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory },
  /* SMEM */
  { 0x00100000, 0x00100000, EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES, 
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType },
  /* Reserved 1 */
  { 0x00200000, 0x02800000, EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES, 
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType },
  /* Display Reserved */
  { 0x02A00000, 0x00C00000, EFI_RESOURCE_MEMORY_RESERVED, EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE, 
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH, AddMem, EfiMaxMemoryType },
  /* Reserved 2 */
	{ 0x03600000, 0x0E200000, EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES, 
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddMem, EfiReservedMemoryType },

  /* HLOS 1 */
  { 0x11800000, 0x1A800000, EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory },
  /* UEFI FD */
  { 0x2C000000, 0x00200000, EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiBootServicesCode },
  /* HLOS 2 */
  { 0x2C200000, 0x03E00000, EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK, AddMem, EfiConventionalMemory },

  /* Peripheral regions */
  
  /* SDC1 */
  { 0xA0300000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* SDC2 */
  { 0xA0400000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* SDC3 */
  { 0xA0500000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* SDC4 */
  { 0xA0600000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* HSUSB */
  { 0xA0800000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* VFE */
  { 0xA0F00000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* SSBI */
  { 0xA8100000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* AXI */
  { 0xA8200000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* IMEM */
  { 0xA8500000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* CLK_CTL */
  { 0xA8600000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* SCPLL */
  { 0xA8800000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* GPIO CFG1 */
  { 0xA8E00000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* GPIO CFG2 */
  { 0xA8F00000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* GPIO1 */
  { 0xA9000000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* GPIO2 */
  { 0xA9100000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* DMOV */
  { 0xA9700000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* I2C */
  { 0xA9900000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* UART1 */
  { 0xA9A00000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* UART2 */
  { 0xA9B00000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* UART3 */
  { 0xA9C00000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* MDP */
  { 0xAA200000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* CLK_CTL_SH2 */
  { 0xABA01000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* VIC */
  { 0xAC000000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },
  /* CSR/GPT */
  { 0xAC100000, 0x00100000, EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE, AddDev, EfiMemoryMappedIO },

  /* Terminator */
  { 0 }
};

#endif