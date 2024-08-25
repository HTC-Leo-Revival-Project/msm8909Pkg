typedef ulong lbaint_t;

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

#define IO_WRITE32(v, a) MmioWrite32((UINT32)(v), (UINTN)(a))
#define IO_READ32(a) MmioRead32((UINTN)(a))