#ifndef _PROTOCOL_PMIC_H__
#define _PROTOCOL_PMIC_H__

#define PMIC_PROTOCOL_GUID                                                     \
  {                                                                            \
    0x2c889818, 0x44c1, 0x4309,                                                \
    {                                                                          \
      0x89, 0x8a, 0x2f, 0x55, 0xc8, 0xcf, 0x0c, 0x87                           \
    }                                                                          \
  }

typedef struct _PMIC_PROTOCOL PMIC_PROTOCOL;

// typedef UINT32(*msm_ssbi_read_t)(UINT16 addr, UINT8 *buf, UINT32 len);
// typedef UINT32(*msm_ssbi_write_t)(UINT16 addr, UINT8 *buf, UINT32 len);
struct _PMIC_PROTOCOL {

};

extern EFI_GUID gPmicProtocolGuid;

#endif