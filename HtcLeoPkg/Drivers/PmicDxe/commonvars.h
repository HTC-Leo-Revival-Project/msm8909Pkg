#ifndef PMIC_COMMONVARS_H
#define PMIC_COMMONVARS_H

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/Pmic.h>
#include <Protocol/Ssbi.h>

#include <Chipset/pmic_msm7230.h>

extern UINT8 revision;
extern SSBI_PROTOCOL *gSsbi;

extern int pm8058_readb(UINT16 addr, UINT8 *val);
extern int pm8058_hard_reset_config(enum pon_config config);
extern int pm8058_writeb(UINT16 addr, UINT8 val);
extern int pm8xxx_get_irq_stat(int irq);
extern int pm8058_read_irq_stat(int irq);
extern int pm_gpio_init_bank1(void);

#endif