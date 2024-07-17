#ifndef __PROTOCOL_CLOCK_DEVICE_H__
#define __PROTOCOL_CLOCK_DEVICE_H__

#define CLOCK_PROTOCOL_GUID                                            \
  {                                                                            \
    0xb27625b5, 0x0b6c, 0x4614,                                                \
    {                                                                          \
      0xaa, 0x3c, 0x33, 0x13, 0xb5, 0x1d, 0x36, 0x48                           \
    }                                                                          \
  }

typedef struct _EMBEDDED_CLOCK_PROTOCOL EMBEDDED_CLOCK_PROTOCOL;

typedef EFI_STATUS(EFIAPI *CLOCK_ENABLE)(UINTN Id);
typedef VOID(EFIAPI *CLOCK_DISABLE)(UINTN Id);
typedef EFI_STATUS(EFIAPI *CLOCK_SET_RATE)(UINTN Id, UINTN Freq);


struct _EMBEDDED_CLOCK_PROTOCOL {
  CLOCK_ENABLE  ClkEnable;
  CLOCK_DISABLE ClkDisable;
  CLOCK_SET_RATE ClkSetRate;
};

extern EFI_GUID gEmbeddedClockProtocolGuid;

#endif