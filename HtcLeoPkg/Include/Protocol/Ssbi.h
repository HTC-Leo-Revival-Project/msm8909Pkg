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

typedef VOID(*sbbi_test)(VOID);
struct _SSBI_PROTOCOL {
  sbbi_test SsbiTest;
};

extern EFI_GUID gHtcLeoMicropProtocolGuid;

#endif