//
//  QSD8250 Platform specific defines for constructing ACPI tables
//

#ifndef _Platform_H_INCLUDED_
#define _Platform_H_INCLUDED_

#include <IndustryStandard/Acpi50.h>

#define EFI_ACPI_OEM_ID           {'Q','C','8','2','5','0'}   // OEMID 6 bytes long
#define EFI_ACPI_OEM_TABLE_ID     SIGNATURE_64('L','E','O','E','D','K','2',' ') // OEM table id 8 bytes long
#define EFI_ACPI_OEM_REVISION     0x02000820
#define EFI_ACPI_CREATOR_ID       SIGNATURE_32('L','E','O',' ')
#define EFI_ACPI_CREATOR_REVISION 0x00000097

#define EFI_ACPI_VENDOR_ID		SIGNATURE_32('Q','C','O','M')
#define EFI_ACPI_CSRT_REVISION	0x00000005
#define EFI_ACPI_CSRT_DEVICE_ID_DMA 0x00000009	// fixed id
#define EFI_ACPI_CSRT_RESOURCE_ID_IN_DMA_GRP	0x0 // count up from 0

#define EFI_ACPI_5_0_CSRT_REVISION 0x00000000

typedef enum 
{
	EFI_ACPI_CSRT_RESOURCE_TYPE_RESERVED,		// 0
	EFI_ACPI_CSRT_RESOURCE_TYPE_INTERRUPT,		// 1
	EFI_ACPI_CSRT_RESOURCE_TYPE_TIMER,			// 2
	EFI_ACPI_CSRT_RESOURCE_TYPE_DMA,			// 3
	EFI_ACPI_CSRT_RESOURCE_TYPE_CACHE,			// 4
}
CSRT_RESOURCE_TYPE;

typedef enum 
{
	EFI_ACPI_CSRT_RESOURCE_SUBTYPE_DMA_CHANNEL,			// 0
	EFI_ACPI_CSRT_RESOURCE_SUBTYPE_DMA_CONTROLLER		// 1
}
CSRT_DMA_SUBTYPE;

//------------------------------------------------------------------------
// CSRT Resource Group header 24 bytes long
//------------------------------------------------------------------------
typedef struct
{
	UINT32 Length;			// Length
	UINT32 VendorID;		// was UINT8 VendorID[4]; 		// 4 bytes
	UINT32 SubVendorId;		// 4 bytes
	UINT16 DeviceId;		// 2 bytes
	UINT16 SubdeviceId;		// 2 bytes
	UINT16 Revision;		// 2 bytes
	UINT16 Reserved;		// 2 bytes
	UINT32 SharedInfoLength;	// 4 bytes
} EFI_ACPI_5_0_CSRT_RESOURCE_GROUP_HEADER;

//------------------------------------------------------------------------
// CSRT Resource Descriptor 12 bytes total
//------------------------------------------------------------------------
typedef struct
{
	UINT32 Length;			// 4 bytes
	UINT16 ResourceType;	// 2 bytes
	UINT16 ResourceSubType;	// 2 bytes
	UINT32 UID;				// 4 bytes    
} EFI_ACPI_5_0_CSRT_RESOURCE_DESCRIPTOR_HEADER;

//------------------------------------------------------------------------
// DBG2 definitions
//------------------------------------------------------------------------
#define USB_NAME_SPACE_STRING_LENGTH  sizeof("\\_SB.UFN1")
#define KD_USB_BASE_ADDR              0xA0800000
#define KD_USB_ADDR_SIZE              0x000001AF
#define KD_USB_ACPI_PATH              "\\_SB.UFN1"

#endif // of _Platform_H_INCLUDED_