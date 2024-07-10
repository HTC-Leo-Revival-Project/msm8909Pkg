typedef enum {
  UsbDeviceEventDeviceStateChange,
  UsbDeviceEventTransferNotification
} USB_DEVICE_EVENT;

typedef enum {
  UsbDeviceTransferStatusCompleted,
  UsbDeviceTransferStatusFailed,
  UsbDeviceTransferStatusAborted,
} USB_DEVICE_TRANSFER_STATUS;

typedef enum { 
  UsbDeviceStateConnected,
  UsbDeviceStateDisconnected
} USB_DEVICE_STATE;

typedef struct {
  USB_DEVICE_TRANSFER_STATUS    Status;
  UINT8                         EndpointIndex;
  UINTN                         Size;
  VOID                          *Buffer;
} USB_DEVICE_TRANSFER_OUTCOME;

typedef union {
  USB_DEVICE_STATE              DeviceState;
  USB_DEVICE_TRANSFER_OUTCOME   TransferOutcome;
} USB_DEVICE_EVENT_DATA;

typedef
VOID
(*USB_DEVICE_EVENT_CALLBACK) (
  IN USB_DEVICE_EVENT         Event,
  IN USB_DEVICE_EVENT_DATA    *EventData
  );