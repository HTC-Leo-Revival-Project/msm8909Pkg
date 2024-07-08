#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/TimerLib.h>

#include <Library/pcom.h>
#include <Library/pcom_clients.h>

#include <Chipset/irqs.h>
#include <Chipset/iomap.h>
#include <Chipset/clock.h>

#include <Protocol/UsbDevice.h>
#include <Protocol/HardwareInterrupt.h>
#include <Protocol/EmbeddedClock.h>

#include "UsbDxe.h"

// Cached copy of the Hardware Interrupt protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

// Cached copy of the Embedded Clock protocol instance
EMBEDDED_CLOCK_PROTOCOL  *gClock = NULL;

// Cached interrupt vector
volatile UINTN  gVector;

STATIC USB_DEVICE_DESCRIPTOR    *mDeviceDescriptor;

// The config descriptor, interface descriptor, and endpoint descriptors in a
// buffer (in that order)
STATIC VOID                     *mDescriptors;
// Convenience pointers to those descriptors inside the buffer:
STATIC USB_INTERFACE_DESCRIPTOR *mInterfaceDescriptor;
STATIC USB_CONFIG_DESCRIPTOR    *mConfigDescriptor;
STATIC USB_ENDPOINT_DESCRIPTOR  *mEndpointDescriptors;

STATIC USB_DEVICE_RX_CALLBACK   mDataReceivedCallback;
STATIC USB_DEVICE_TX_CALLBACK   mDataSentCallback;

/* Private endpoint-related variables */
STATIC UINTN EndpointBit; // Endpoint bit
STATIC UINTN EndpointIn;  // Endpoint direction; in = 1
STATIC UINT8 EndpointIndex = 1;
STATIC UINTN UsbHighspeed; //HS = 1

//https://relay.vvl.me:4443/acrn-edk2/ovmf-acrn-v1.2/source/EmbeddedPkg/Drivers/Isp1761UsbDxe/Isp1761UsbDxe.c

/*
 * Note: This Protocol is just  the bare minimum for Android Fastboot. It
 * only makes sense for devices that only do Bulk Transfers and only have one
 * endpoint.
 */

// Go to the Status stage of a successful control transfer
STATIC
VOID
StatusAcknowledge ()
{
  MmioWrite32(USB_ENDPTPRIME, EndpointBit);
}

// Read the FIFO for the endpoint indexed by Endpoint, into the buffer pointed
// at by Buffer, whose size is *Size bytes.
//
// If *Size is less than the number of bytes in the FIFO, return EFI_BUFFER_TOO_SMALL
//
// Update *Size with the number of bytes of data in the FIFO.
/*STATIC
EFI_STATUS
ReadEndpointBuffer (
  IN      UINT8   Endpoint,
  IN OUT  UINTN  *Size,
  IN OUT  VOID   *Buffer
  )
{
  return EFI_SUCCESS;
}*/

/*
  Write an endpoint buffer. Parameters:
  Endpoint        Endpoint index (see Endpoint Index Register in datasheet)
  MaxPacketSize   The MaxPacketSize this endpoint is configured for
  Size            The size of the Buffer
  Buffer          The data

  Assumes MaxPacketSize is a multiple of 4.
  (It seems that all valid values for MaxPacketSize _are_ multiples of 4)
*/
/*
STATIC
EFI_STATUS
WriteEndpointBuffer (
  IN       UINT8   Endpoint,
  IN       UINTN   MaxPacketSize,
  IN       UINTN   Size,
  IN CONST VOID   *Buffer
  )
{
  return EFI_SUCCESS;
}*/

STATIC
EFI_STATUS
HandleGetDescriptor (
  IN USB_DEVICE_REQUEST  *Request
  )
{
  EFI_STATUS  Status;
  UINT8       DescriptorType;
  UINTN       ResponseSize;
  VOID       *ResponseData;

  ResponseSize = 0;
  ResponseData = NULL;
  Status = EFI_SUCCESS;

  // Pretty confused if bmRequestType is anything but this:
  ASSERT (Request->RequestType == USB_DEV_GET_DESCRIPTOR_REQ_TYPE);

  // Choose the response
  DescriptorType = Request->Value >> 8;
  switch (DescriptorType) {
  case USB_DESC_TYPE_DEVICE:
    DEBUG ((EFI_D_INFO, "USB: Got a request for device descriptor\n"));
    ResponseSize = sizeof (USB_DEVICE_DESCRIPTOR);
    ResponseData = mDeviceDescriptor;
    break;
  case USB_DESC_TYPE_CONFIG:
    DEBUG ((EFI_D_INFO, "USB: Got a request for config descriptor\n"));
    ResponseSize = mConfigDescriptor->TotalLength;
    ResponseData = mDescriptors;
    break;
  case USB_DESC_TYPE_STRING:
    DEBUG ((EFI_D_INFO, "USB: Got a request for String descriptor %d\n", Request->Value & 0xFF));
    break;
  default:
    DEBUG ((EFI_D_INFO, "USB: Didn't understand request for descriptor 0x%04x\n", Request->Value));
    Status = EFI_NOT_FOUND;
    break;
  }

  // Send the response
  if (ResponseData) {
    ASSERT (ResponseSize != 0);

    if (Request->Length < ResponseSize) {
      // Truncate response
      ResponseSize = Request->Length;
    } else if (Request->Length > ResponseSize) {
      DEBUG ((EFI_D_INFO, "USB: Info: ResponseSize < wLength\n"));
    }

    /*DataStageEnable (ISP1761_EP0TX);
    Status = WriteEndpointBuffer (
              ISP1761_EP0TX,
              MAX_PACKET_SIZE_CONTROL,
              ResponseSize,
              ResponseData
              );*/
    if (!EFI_ERROR (Status)) {
      // Setting this value should cause us to go to the Status stage on the
      // next EP0TX interrupt
      //mControlTxPending = TRUE;
    }
  }

  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
HandleSetAddress (
  IN USB_DEVICE_REQUEST  *Request
  )
{  
  DEBUG ((EFI_D_INFO, "USB: Setting address to %d\n", Request->Value));
  MmioWrite32 (USB_DEVICEADDR, (Request->Value << 25) | (1 << 24));
  StatusAcknowledge();

  return EFI_SUCCESS;
}

STATIC
VOID
EndpointEnable (
  IN BOOLEAN Enable
)
{
  UINTN n;

  n = MmioRead32(USB_ENDPTCTRL(EndpointIndex));

  if(Enable) {
    if(EndpointIn) {
      n |= (CTRL_TXE | CTRL_TXR | CTRL_TXT_BULK);
    }
    else {
      n |= (CTRL_RXE | CTRL_RXR | CTRL_RXT_BULK);
    }

    if(EndpointIndex != 0) {
      // HS: ept->head->config = CONFIG_MAX_PKT(512) | CONFIG_ZLT;
      //ept->head->config = CONFIG_MAX_PKT(64) | CONFIG_ZLT;
    }
  }
  MmioWrite32(USB_ENDPTCTRL(EndpointIndex), n);
}

// Move the device to the Configured state.
// (This code only supports one configuration for a device, so the configuration
//  index is ignored)
STATIC
EFI_STATUS
HandleSetConfiguration (
  IN USB_DEVICE_REQUEST  *Request
  )
{
  USB_ENDPOINT_DESCRIPTOR  *EPDesc;
  UINTN                     Index;

  ASSERT (Request->RequestType == USB_DEV_SET_CONFIGURATION_REQ_TYPE);
  DEBUG ((EFI_D_INFO, "USB: Setting configuration.\n"));

  // Configure endpoints
  for (Index = 0; Index < mInterfaceDescriptor->NumEndpoints; Index++) {
    if (Index == 0) {
      EPDesc = &mEndpointDescriptors[Index];
      //EndpointEnable??
      EndpointEnable(TRUE);

      //Set endpoint type (Bulk/Isochronous/Interrupt)
      //MmioWrite32 (ENDPOINT_MAX_PACKET_SIZE, EPDesc->MaxPacketSize);
    }
  }

  StatusAcknowledge();
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
HandleDeviceRequest (
  IN USB_DEVICE_REQUEST  *Request
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  MmioWrite32(USB_ENDPTSETUPSTAT, EndpointBit);//out = 0, in = 1

  switch (Request->Request) {
  case USB_DEV_GET_DESCRIPTOR:
    Status = HandleGetDescriptor (Request);
    break;
  case USB_DEV_SET_ADDRESS:
    Status = HandleSetAddress (Request);
    break;
  case USB_DEV_SET_CONFIGURATION:
    Status = HandleSetConfiguration (Request);
    break;
  default:
    DEBUG ((EFI_D_ERROR,
      "Didn't understand RequestType 0x%x Request 0x%x\n",
      Request->RequestType, Request->Request));
      Status = EFI_INVALID_PARAMETER;
    break;
  }
  return Status;
}

/* 
 Check if USB cable is connected
 
 @retval 1           Usb cable is connected.
 @retval 0           Usb cable is disconnected.
*/
UINTN
IsUsbConnected ()
{
    /* Verify B Session Valid Bit to verify vbus status */
    if (B_SESSION_VALID & MmioRead32(USB_OTGSC)) {
        return 1;
    } else {
        return 0;
    }
}


/* Saved for reference
https://github.com/u-boot/u-boot/blob/master/drivers/usb/gadget/ci_udc.c#L840
*/
VOID
EFIAPI
UdcInterruptHandler (
  IN  HARDWARE_INTERRUPT_SOURCE   Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext
)
{
  UINTN n, speed;
  USB_DEVICE_REQUEST Request;

  DEBUG((EFI_D_ERROR, "USB INTERRUPT\n"));

  n = MmioRead32(USB_USBSTS);
  MmioWrite32(USB_USBSTS, n);

  n &= (STS_SLI | STS_URI | STS_PCI | STS_UI | STS_UEI);

  if (n == 0) {
    DEBUG((EFI_D_ERROR, "Ouch, this shouldn't be like that....\n"));
    goto end;
  }

  if (n & STS_URI) {
    DEBUG((EFI_D_ERROR, "Reset\n"));

    MmioWrite32(USB_ENDPTCOMPLETE, MmioRead32(USB_ENDPTCOMPLETE));
    MmioWrite32(USB_ENDPTSETUPSTAT, MmioRead32(USB_ENDPTSETUPSTAT));
    MmioWrite32(USB_ENDPTFLUSH, 0xffffffff);
    MmioWrite32(USB_ENDPTCTRL(1), 0);

    /* TODO: */
    /* Notify gadget of offline */
    /* error out any pending reqs */

    StatusAcknowledge();
  }

  if (n & STS_SLI) {
    DEBUG((EFI_D_ERROR, "Suspend\n"));
	}

  if (n & STS_PCI) {
    DEBUG((EFI_D_ERROR, "Portchange\n"));
    speed = (MmioRead32(USB_PORTSC) >> 26) & 3;
    if (speed == 2) {
      UsbHighspeed = 1;
      DEBUG((EFI_D_ERROR, "Usb is HS\n"));
    }
    else {
      UsbHighspeed = 0;
      DEBUG((EFI_D_ERROR, "Usb is not HS\n"));
    }
    /*if(IsUsbConnected()) {
      set_charger_state(usb_config_value ? CHG_USB_HIGH : CHG_USB_LOW);
    }*/
  }

  if (n & STS_UEI) {
    DEBUG((EFI_D_ERROR, "UEI\n"));
	}

  // TODO: Make this code actually usefull
  if ((n & STS_UI) || (n & STS_UEI)) {
    DEBUG((EFI_D_ERROR, "2\n"));
    n = MmioRead32(USB_ENDPTSETUPSTAT);
    if (n & EPT_RX(0)) {
			//handle_setup(ep0out);//0
      /*
      UINT8     RequestType;
      UINT8     Request;
      UINT16    Value;
      UINT16    Index;
      UINT16    Length;
      */
      DEBUG((EFI_D_ERROR, "READ DESCRIPTOR!!!!"));
      HandleDeviceRequest(&Request);
		}

    n = MmioRead32(USB_ENDPTCOMPLETE);
    if(n != 0) {
      DEBUG((EFI_D_ERROR, "3\n"));
      MmioWrite32(USB_ENDPTCOMPLETE, n);
    }

    StatusAcknowledge();
    DEBUG((EFI_D_ERROR, "4\n"));
  }
  end:
  DEBUG((EFI_D_ERROR, "USB INTERRUPT END\n"));
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
  DEBUG((EFI_D_ERROR, "UsbDeviceSend()\n"));
    return EFI_SUCCESS;/*WriteEndpointBuffer (
          EndpointIndex,
          MAX_PACKET_SIZE_BULK,
          Size,
          Buffer
          );*/
}

static inline void OtgXceivReset()
{
	gClock->ClkDisable(USB_HS_CLK);
	gClock->ClkDisable(USB_HS_PCLK);
	MicroSecondDelay(20);

	gClock->ClkEnable(USB_HS_PCLK);
	gClock->ClkEnable(USB_HS_CLK);
	MicroSecondDelay(20);
}

VOID
UdcReset() 
{
	OtgXceivReset();
	//ulpi_set_power(true);
	MicroSecondDelay(100);
	OtgXceivReset();
	
	/* disable usb interrupts and otg */
	MmioWrite32(USB_OTGSC, 0);
	MicroSecondDelay(5);
	
	/* select ULPI phy */
	MmioWrite32(USB_PORTSC, 0x81000000);
	
	/* RESET */
	MmioWrite32(USB_USBCMD, 0x80002);
	MicroSecondDelay(20);

	MmioWrite32(USB_ENDPOINTLISTADDR, (unsigned) mEndpointDescriptors);
	
	/* select DEVICE mode */
  MmioWrite32(USB_USBMODE, 0x02);

  MmioWrite32(USB_ENDPTFLUSH, 0xffffffff);
  MicroSecondDelay(20);
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
{
  UINT8                    *Ptr;
  EFI_STATUS                Status;

  ASSERT (DeviceDescriptor != NULL);
  ASSERT (Descriptors[0] != NULL);
  ASSERT (RxCallback != NULL);
  ASSERT (TxCallback != NULL);

  // Find the clock controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gEmbeddedClockProtocolGuid, NULL, (VOID **)&gClock);
  ASSERT_EFI_ERROR (Status);

  // ------------------- UDC_INIT ------------------

  UdcReset();

  if (IsUsbConnected == 0) {
    DEBUG ((EFI_D_ERROR, "USB: Session not valid.\n"));
  }

  /*
    if(EndpointIn) {
        EndpointBit = EPT_TX(EndpointIndex);
    } else {
        EndpointBit = EPT_RX(EndpointIndex);
        if(num == 0) 
            cfg |= CONFIG_IOS;
    }*/

  // ------------------- UDC_INIT END ------------------ //

  // ------------------- UDC_START ------------------ //

  mDeviceDescriptor = DeviceDescriptor;
  mDescriptors = Descriptors[0];

  // Right now we just support one configuration
  ASSERT (mDeviceDescriptor->NumConfigurations == 1);
  // ... and one interface
  mConfigDescriptor = (USB_CONFIG_DESCRIPTOR *)mDescriptors;
  ASSERT (mConfigDescriptor->NumInterfaces == 1);

  Ptr = ((UINT8 *) mDescriptors) + sizeof (USB_CONFIG_DESCRIPTOR);
  mInterfaceDescriptor = (USB_INTERFACE_DESCRIPTOR *) Ptr;
  Ptr += sizeof (USB_INTERFACE_DESCRIPTOR);

  mEndpointDescriptors = (USB_ENDPOINT_DESCRIPTOR *) Ptr;

  mDataReceivedCallback = RxCallback;
  mDataSentCallback = TxCallback;

  /* go to RUN mode (D+ pullup enable) */
	MmioWrite32(USB_USBCMD, 0x00080001);

  // Find the interrupt controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  ASSERT_EFI_ERROR (Status);

  gVector = INT_USB_HS;

  // Install interrupt handler
  Status = gInterrupt->RegisterInterruptSource (gInterrupt, gVector, UdcInterruptHandler);
  ASSERT_EFI_ERROR (Status);

  MmioWrite32(USB_USBINTR, STS_URI | STS_SLI | STS_UI | STS_PCI);

  return Status;
}

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
  EFI_HANDLE  Handle = NULL;

  // Install the USB Device Protocol onto a new handle
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gUsbDeviceProtocolGuid,      
                  &mUsbDevice,
                  NULL
                  );

  ASSERT_EFI_ERROR(Status);

  return Status;
}