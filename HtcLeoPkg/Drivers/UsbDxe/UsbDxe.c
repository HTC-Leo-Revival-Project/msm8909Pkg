#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>

#include <Protocol/UsbDevice.h>

#include "UsbDxe.h"

//https://relay.vvl.me:4443/acrn-edk2/ovmf-acrn-v1.2/source/EmbeddedPkg/Drivers/Isp1761UsbDxe/Isp1761UsbDxe.c

/*
 * Note: This Protocol is just  the bare minimum for Android Fastboot. It
 * only makes sense for devices that only do Bulk Transfers and only have one
 * endpoint.
 */

/*
  Write an endpoint buffer. Parameters:
  Endpoint        Endpoint index (see Endpoint Index Register in datasheet)
  MaxPacketSize   The MaxPacketSize this endpoint is configured for
  Size            The size of the Buffer
  Buffer          The data

  Assumes MaxPacketSize is a multiple of 4.
  (It seems that all valid values for MaxPacketSize _are_ multiples of 4)
*/
STATIC
EFI_STATUS
WriteEndpointBuffer (
  IN       UINT8   Endpoint,
  IN       UINTN   MaxPacketSize,
  IN       UINTN   Size,
  IN CONST VOID   *Buffer
  )
{

}

/*
  Put data in the Tx buffer to be sent on the next IN token.
  Don't call this function again until the TxCallback has been called.

  @param[in]Endpoint    Endpoint index, as specified in endpoint descriptors, of
                        the endpoint to send the data from.
  @param[in]Size        Size in bytes of data.
  @param[in]Buffer      Pointer to data.

  @retval EFI_SUCCESS           The data was queued successfully.
  @retval EFI_INVALID_PARAMETER There was an error sending the data.
*/
EFI_STATUS
UsbDeviceSend (
  IN       UINT8  EndpointIndex,
  IN       UINTN  Size,
  IN CONST VOID   *Buffer
)
{
    // Device-specific
    /*
    void ulpi_write(unsigned val, unsigned reg)
    {
        // initiate write operation
        writel(ULPI_RUN | ULPI_WRITE | 
                ULPI_ADDR(reg) | ULPI_DATA(val),
                USB_ULPI_VIEWPORT);

        //wait for completion
        while(readl(USB_ULPI_VIEWPORT) & ULPI_RUN) ;
    }
    */
    
    /*
    return WriteEndpointBuffer (
          (EndpointIndex << 1) | 0x1, //Convert to ISP1761 endpoint index, Tx
          MAX_PACKET_SIZE_BULK,
          Size,
          Buffer
          );
    */
}

/*
  Restart the USB peripheral controller and respond to enumeration.

  @param[in] DeviceDescriptor   pointer to device descriptor
  @param[in] Descriptors        Array of pointers to buffers, where
                                Descriptors[n] contains the response to a
                                GET_DESCRIPTOR request for configuration n. From
                                USB Spec section 9.4.3:
                                "The first interface descriptor follows the
                                configuration descriptor. The endpoint
                                descriptors for the first interface follow the
                                first interface descriptor. If there are
                                additional interfaces, their interface
                                descriptor and endpoint descriptors follow the
                                first interface’s endpoint descriptors".

                                The size of each buffer is the TotalLength
                                member of the Configuration Descriptor.

                                The size of the array is
                                DeviceDescriptor->NumConfigurations.
  @param[in]RxCallback          See USB_DEVICE_RX_CALLBACK
  @param[in]TxCallback          See USB_DEVICE_TX_CALLBACK
*/
EFI_STATUS
UsbDeviceStart (
  IN USB_DEVICE_DESCRIPTOR   *DeviceDescriptor,
  IN VOID                    **Descriptors,
  IN USB_DEVICE_RX_CALLBACK  RxCallback,
  IN USB_DEVICE_TX_CALLBACK  TxCallback
)

USB_DEVICE_PROTOCOL mUsbDevice = {
  UsbDeviceStart,
  UsbDeviceSend
};

EFI_STATUS
EFIAPI
UsbDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  // Initialize the USB device

  // Install the USB Device Protocol onto a new handle
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gUsbDeviceProtocolGuid,      
                  &gUsb,
                  NULL
                  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}