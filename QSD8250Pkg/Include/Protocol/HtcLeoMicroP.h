#ifndef __HTCLEO_ROTOCOL_I2C_MICROP_H__
#define __HTCLEO_ROTOCOL_I2C_MICROP_H__


#define HTCLEO_MICROP_PROTOCOL_GUID                                             \
  {                                                                            \
    0x2c898318, 0x41c1, 0x4309,                                                \
    {                                                                          \
      0x89, 0x8a, 0x2f, 0x55, 0xc8, 0xcf, 0x0b, 0x86                           \
    }                                                                          \
  }

typedef struct _HTCLEO_MICROP_PROTOCOL HTCLEO_MICROP_PROTOCOL;

typedef INTN(*microp_i2c_write_t)(UINT8 addr, UINT8 *cmd, INTN length);
typedef INTN(*microp_i2c_read_t)(UINT8 addr, UINT8 *data, INTN length);
typedef VOID(*microp_led_set_mode_t)(UINT8 mode);
#if DEVICETYPE == 3
typedef VOID(*microp_kp_led_set_brightness_t)(UINT8 brightness);
#endif
#if DEVICETYPE == 4
typedef VOID(*trackball_led_set_mode_t)(int rpwm, int gpwm, int bpwm, int brightness, int period);
#endif

struct _HTCLEO_MICROP_PROTOCOL {
  microp_i2c_write_t  Write;
  microp_i2c_read_t Read;
  microp_led_set_mode_t LedSetMode;
#if DEVICETYPE == 3
  microp_kp_led_set_brightness_t KpLedSetBrightness;
#endif
#if DEVICETYPE == 4
  trackball_led_set_mode_t JogBallLedSetColor;
#endif
};

extern EFI_GUID gHtcLeoMicropProtocolGuid;

#endif