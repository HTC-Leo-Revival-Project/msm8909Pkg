#ifndef __LIBRARY_HTCLEO_PLATFORM_LIB_H__
#define __LIBRARY_HTCLEO_PLATFORM_LIB_H__

VOID EFIAPI reboot (unsigned rebootReason);
VOID EFIAPI htcleo_reboot (unsigned rebootReason);
VOID EFIAPI ResetShutdown (VOID);
VOID EFIAPI ResetCold (VOID);

#endif
