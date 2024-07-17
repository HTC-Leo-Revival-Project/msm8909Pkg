#ifndef __LIBRARY_LKENV_H__
#define __LIBRARY_LKENV_H__

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/ArmLib.h>

typedef INT8 int8_t;
typedef INT16 int16_t;
typedef INT32 int32_t;
typedef INT64 int64_t;

typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;

typedef UINT8 u8;
typedef UINT16 u16;
typedef UINT32 u32;
typedef UINT64 u64;

typedef UINTN size_t;
typedef BOOLEAN bool;
typedef BOOLEAN status_t;
typedef UINTN addr_t;
typedef UINTN paddr_t;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef UINTN uint;
typedef unsigned long ulong;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef UINTN u_int;
typedef unsigned long u_long;
typedef ulong lbaint_t;

#define true TRUE
#define false FALSE

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
	((type *)((UINT32)(ptr) - offsetof(type, member)))
#endif

#ifndef ROUND_TO_PAGE
#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#endif

#define ERR_INVALID_ARGS -8

#define UINT_MAX MAX_UINTN

#define IO_WRITE32(v, a) MmioWrite32((UINT32)(v), (UINTN)(a))
#define IO_READ32(a) MmioRead32((UINTN)(a))

#define BIT(bit) (1U << (bit))
#define BIT_GET(x, bit) ((x)& (1 << (bit)))
#define BIT_SHIFT(x, bit) (((x) >> (bit)) & 1)
#define BITS(x, high, low) ((x)& (((1 << ((high)+1)) - 1) & ~((1 << (low)) - 1)))

#define NO_ERROR 0
#define ERROR -1
#define ERR_NOT_FOUND -2
#define ERR_NOT_READY -3
#define ERR_NO_MSG -4
#define ERR_NO_MEMORY -5
#define ERR_ALREADY_STARTED -6
#define ERR_NOT_VALID -7
#define ERR_INVALID_ARGS -8
#define ERR_NOT_ENOUGH_BUFFER -9
#define ERR_NOT_SUSPENDED -10
#define ERR_OBJECT_DESTROYED -11
#define ERR_NOT_BLOCKED -12
#define ERR_TIMED_OUT -13
#define ERR_ALREADY_EXISTS -14
#define ERR_CHANNEL_CLOSED -15
#define ERR_OFFLINE -16
#define ERR_NOT_ALLOWED -17
#define ERR_BAD_PATH -18
#define ERR_ALREADY_MOUNTED -19
#define ERR_IO -20
#define ERR_NOT_DIR -21
#define ERR_NOT_FILE -22
#define ERR_RECURSE_TOO_DEEP -23
#define ERR_NOT_SUPPORTED -24
#define ERR_TOO_BIG -25

/*#define strcmp(s1, s2) ((int)AsciiStrCmp((s1), (s2)))
#define strncmp(s1, s2, n) ((int)AsciiStrnCmp((s1), (s2), (n)))
#define strlen(s) ((size_t)AsciiStrLen((s)))
#define strlcpy(dst, src, n) AsciiStrCpyS((dst), (n), (src))
#define strstr(s1 , s2) AsciiStrStr((s1), (s2))
#define memset(s, c, n) SetMem((s), (UINTN)(n), (UINT8)(c))
#define memcpy(s1, s2, n) CopyMem((s1), (s2), (n))
#define memmove(s1, s2, n) CopyMem((s1), (s2), (n))
#define memcmp(s1, s2, n) ((int)CompareMem((s1), (s2), (n)))

#define va_list VA_LIST
//#define offsetof(type, member) OFFSET_OF (type, member)
#define __PACKED __attribute__((packed))

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))
#define CACHE_LINE (ArmDataCacheLineLength())
#define IS_CACHE_LINE_ALIGNED(addr)  !((UINTN) (addr) & (CACHE_LINE - 1))

#define snprintf(s, n, fmt, ...) ((int)AsciiSPrint((s), (n), (fmt), ##__VA_ARGS__))

// debug levels
#define CRITICAL DEBUG_ERROR
#define ALWAYS DEBUG_ERROR
#define INFO DEBUG_INFO
#define SPEW DEBUG_VERBOSE
*/
#if !defined(MDEPKG_NDEBUG)
#define debug(fmt, ...) do { \
                                   if (DebugPrintEnabled ()) { \
                                     CHAR8 __printbuf[100]; \
                                     UINTN __printindex; \
                                     CONST CHAR8 *__fmtptr = (fmt); \
                                     UINTN __fmtlen = AsciiStrSize(__fmtptr); \
                                     CopyMem(__printbuf, __fmtptr, __fmtlen); \
                                     __printbuf[__fmtlen-1] = 0; \
                                     for(__printindex=1; __printbuf[__printindex]; __printindex++) { \
                                       if (__printbuf[__printindex-1]=='%' && __printbuf[__printindex]=='s') \
                                         __printbuf[__printindex] = 'a'; \
                                     } \
                                     DEBUG(((EFI_D_ERROR), __printbuf, ##__VA_ARGS__)); \
                                   } \
                                 } while(0)                              
#define printf(fmt, ...) do { \
                                   if (DebugPrintEnabled ()) { \
                                     CHAR8 __printbuf[100]; \
                                     UINTN __printindex; \
                                     CONST CHAR8 *__fmtptr = (fmt); \
                                     UINTN __fmtlen = AsciiStrSize(__fmtptr); \
                                     CopyMem(__printbuf, __fmtptr, __fmtlen); \
                                     __printbuf[__fmtlen-1] = 0; \
                                     for(__printindex=1; __printbuf[__printindex]; __printindex++) { \
                                       if (__printbuf[__printindex-1]=='%' && __printbuf[__printindex]=='s') \
                                         __printbuf[__printindex] = 'a'; \
                                     } \
                                     DEBUG(((EFI_D_ERROR), __printbuf, ##__VA_ARGS__)); \
                                   } \
                                 } while(0)   
#define sprintf(fmt, ...) do { \
                                   if (DebugPrintEnabled ()) { \
                                     CHAR8 __printbuf[100]; \
                                     UINTN __printindex; \
                                     CONST CHAR8 *__fmtptr = (fmt); \
                                     UINTN __fmtlen = AsciiStrSize(__fmtptr); \
                                     CopyMem(__printbuf, __fmtptr, __fmtlen); \
                                     __printbuf[__fmtlen-1] = 0; \
                                     for(__printindex=1; __printbuf[__printindex]; __printindex++) { \
                                       if (__printbuf[__printindex-1]=='%' && __printbuf[__printindex]=='s') \
                                         __printbuf[__printindex] = 'a'; \
                                     } \
                                     DEBUG(((EFI_D_ERROR), __printbuf, ##__VA_ARGS__)); \
                                   } \
                                 } while(0)   
#else 
#define sprintf(fmt, ...)
#define printf(fmt, ...)
#define debug(fmt, ...)
#endif

#define DMB ArmDataMemoryBarrier()
#define DSB ArmDataSynchronizationBarrier()

#define __ALWAYS_INLINE __attribute__ ((always_inline))

/* Defines added from qcom_lk.h */

// define a macro that unconditionally swaps
#define SWAP_32(x) \
	(((UINT32)(x) << 24) | (((UINT32)(x) & 0xff00) << 8) |(((UINT32)(x) & 0x00ff0000) >> 8) | ((UINT32)(x) >> 24))
#define SWAP_16(x) \
	((((UINT16)(x) & 0xff) << 8) | ((UINT16)(x) >> 8))
	
#define LE32(val) (val)
#define LE16(val) (val)
#define BE32(val) SWAP_32(val)
#define BE16(val) SWAP_16(val)

#define LE32SWAP(var) (var) = LE32(var);
#define LE16SWAP(var) (var) = LE16(var);
#define BE32SWAP(var) (var) = BE32(var);
#define BE16SWAP(var) (var) = BE16(var);

/* low level macros for accessing memory mapped hardware registers */
#define REG64(addr) ((volatile UINT64 *)(addr))
#define REG32(addr) ((volatile UINT32 *)(addr))
#define REG16(addr) ((volatile UINT16 *)(addr))
#define REG8(addr) ((volatile UINT8 *)(addr))

#define RMWREG64(addr, startbit, width, val) *REG64(addr) = (*REG64(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG16(addr, startbit, width, val) *REG16(addr) = (*REG16(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG8(addr, startbit, width, val) *REG8(addr) = (*REG8(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))

#define __UNUSED //added

// Timer
#define mdelay(msecs) MicroSecondDelay((msecs)*1000)
#define udelay(usecs) MicroSecondDelay((usecs))

#endif

/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef _U_BOOT_H_
#define _U_BOOT_H_	1

#define CONFIG_NR_DRAM_BANKS 3

typedef struct bd_info {
    int			bi_baudrate;	/* serial console baudrate */
    unsigned long	bi_ip_addr;	/* IP Address */
    struct environment_s	       *bi_env;
    ulong	        bi_arch_number;	/* unique id for this board */
    ulong	        bi_boot_params;	/* where this board expects params */
    struct				/* RAM configuration */
    {
	ulong start;
	ulong size;
    }			bi_dram[CONFIG_NR_DRAM_BANKS];
} bd_t;

#define bi_env_data bi_env->data
#define bi_env_crc  bi_env->crc

#endif	/* _U_BOOT_H_ */
