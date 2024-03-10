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
  HobOnlyNoCacheSetting,
	MaxMem
} DeviceMemoryAddHob;

#define MEMORY_REGION_NAME_MAX_LENGTH 32

typedef struct {
  CHAR8                        Name[MEMORY_REGION_NAME_MAX_LENGTH];
	EFI_PHYSICAL_ADDRESS         Address;
	UINT64                       Length;
  DeviceMemoryAddHob           HobOption;
	EFI_RESOURCE_TYPE            ResourceType;
	EFI_RESOURCE_ATTRIBUTE_TYPE	 ResourceAttribute;
	EFI_MEMORY_TYPE              MemoryType;
  ARM_MEMORY_REGION_ATTRIBUTES ArmAttributes;
} ARM_MEMORY_REGION_DESCRIPTOR_EX, *PARM_MEMORY_REGION_DESCRIPTOR_EX;

#define MEM_RES EFI_RESOURCE_MEMORY_RESERVED
#define MMAP_IO EFI_RESOURCE_MEMORY_MAPPED_IO
#define SYS_MEM EFI_RESOURCE_SYSTEM_MEMORY

#define SYS_MEM_CAP SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES
#define INITIALIZED EFI_RESOURCE_ATTRIBUTE_INITIALIZED
#define WRITE_COMBINEABLE EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE
#define UNCACHEABLE EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE

#define Reserv EfiReservedMemoryType
#define Conv EfiConventionalMemory
#define BsData EfiBootServicesData
#define RtData EfiRuntimeServicesData
#define LdData EfiLoaderData
#define MmIO EfiMemoryMappedIO
#define MaxMem EfiMaxMemoryType
#define BsCode EfiBootServicesCode
#define RtCode EfiRuntimeServicesCode

#define NS_DEVICE ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
#define WRITE_THROUGH ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH
#define WRITE_THROUGH_XN ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH
#define WRITE_BACK ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define WRITE_BACK_XN ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define UNCACHED_UNBUFFERED ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED
#define UNCACHED_UNBUFFERED_XN ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

/* Unused, see map in HtcLeoPkgLib */

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] =
{
  /* Name   Address, Length,  HobOption        ResourceAttribute    ArmAttributes  ResourceType          MemoryType */

  /* DDR Regions */
  {"APPSBL",            0x00000000, 0x00100000, AddMem, MEM_RES, UNCACHEABLE,   Reserv,   UNCACHED_UNBUFFERED}, /* Probably shouldn't be hlos, check needed */
  {"SMEM",              0x00100000, 0x00100000, AddMem, MEM_RES, UNCACHEABLE,   Reserv, UNCACHED_UNBUFFERED},
  {"Reserved 1",        0x00200000, 0x02800000, AddMem, SYS_MEM, SYS_MEM_CAP,   Reserv, NS_DEVICE},
  {"Display Reserved",  0x03B00000, 0x000C0000, AddMem, MEM_RES, WRITE_THROUGH, MaxMem, WRITE_THROUGH},
  {"Reserved 2",        0x02AC0000, 0x0ED40000, AddMem, SYS_MEM, SYS_MEM_CAP,   Reserv, NS_DEVICE},
  {"HLOS 0",            0x11800000, 0x1A800000, AddMem, SYS_MEM, SYS_MEM_CAP,   Conv,   WRITE_BACK},
  {"UEFI FD",           0x2C000000, 0x00200000, AddMem, SYS_MEM, SYS_MEM_CAP,   BsCode, WRITE_BACK},
#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  {"HLOS 1",            0x2C200000, 0x03C00000, AddMem, SYS_MEM, SYS_MEM_CAP,   Conv,   WRITE_BACK},
  {"PStore",            0x2FE00000, 0x00200000, AddMem, MEM_RES, SYS_MEM_CAP,   Reserv, WRITE_THROUGH_XN},
#else
  {"HLOS 1",            0x2C200000, 0x03E00000, AddMem, SYS_MEM, SYS_MEM_CAP,   Conv,   WRITE_BACK},
#endif

  /* Peripheral regions */
  {"GPU",               0xA0000000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"TSIF",              0xA0100000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"UART1_DM",          0xA0200000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"SDC1",              0xA0300000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"SDC2",              0xA0400000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"SDC3",              0xA0500000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"SDC4",              0xA0600000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"HSUSB",             0xA0800000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"VFE",               0xA0F00000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"SSBI",              0xA8100000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"AXI",               0xA8200000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"IMEM",              0xA8500000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"CLK_CTL",           0xA8600000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"SCPLL",             0xA8800000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"GPIO CFG1",         0xA8E00000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"GPIO CFG2",         0xA8F00000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"GPIO1",             0xA9000000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"GPIO2",             0xA9100000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"DMOV",              0xA9700000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"I2C",               0xA9900000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"UART1",             0xA9A00000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"UART2",             0xA9B00000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"UART3",             0xA9C00000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"MDP",               0xAA200000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"CLK_CTL_SH2",       0xABA01000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"VIC",               0xAC000000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"CSR/GPT",           0xAC100000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},
  {"SIRC",              0xAC100000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE,   MmIO,   NS_DEVICE},

  /* Terminator for MMU */
  {"Terminator", 0, 0, 0, 0, 0, 0, 0}};

#endif