#ifndef _PROTOCOL_SSBI_H__
#define _PROTOCOL_SSBI_H__

#define SSBI_PROTOCOL_GUID                                                     \
  {                                                                            \
    0x2c889818, 0x44c1, 0x4309,                                                \
    {                                                                          \
      0x89, 0x8a, 0x2f, 0x55, 0xc8, 0xcf, 0x0b, 0x86                           \
    }                                                                          \
  }

typedef struct _SSBI_PROTOCOL SSBI_PROTOCOL;

typedef UINT32(*msm_ssbi_read_t)(UINT16 addr, UINT8 *buf, UINT32 len);
typedef UINT32(*msm_ssbi_write_t)(UINT16 addr, UINT8 *buf, UINT32 len);
struct _SSBI_PROTOCOL {
  msm_ssbi_read_t SsbiRead;
  msm_ssbi_write_t SsbiWrite;
};

extern EFI_GUID gSsbiProtocolGuid;

#endif