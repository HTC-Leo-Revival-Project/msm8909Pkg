#ifndef __HTCLEO_ROTOCOL_I2C_H__
#define __HTCLEO_ROTOCOL_I2C_H__

#include <Chipset/msm_i2c.h>

#define HTCLEO_I2C_PROTOCOL_GUID                                             \
  {                                                                            \
    0x2c898318, 0x41c1, 0x4309,                                                \
    {                                                                          \
      0x89, 0x8a, 0x2f, 0x55, 0xc8, 0xcf, 0x0b, 0x85                           \
    }                                                                          \
  }

typedef struct _HTCLEO_I2C_PROTOCOL HTCLEO_I2C_PROTOCOL;

typedef INTN(*i2c_write_t)(int chip, void *buf, UINTN count);
typedef INTN(*i2c_read_t)(int chip, UINT8 reg, void *buf, UINTN count);
typedef INTN(*i2c_xfer_t)(struct i2c_msg msgs[], int num);

struct _HTCLEO_I2C_PROTOCOL {
  i2c_write_t  Write;
  i2c_read_t Read;
  i2c_xfer_t Xfer;
};

extern EFI_GUID gHtcLeoI2CProtocolGuid;

#endif