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

SSBI_PROTOCOL *gSsbi = NULL;

static int pm8058_readb(UINT16 addr, UINT8 *val)
{

	return gSsbi->SsbiRead(addr, val, 1);
}

static int pm8058_writeb(UINT16 addr, UINT8 val)
{

	return gSsbi->SsbiWrite(addr, &val, 1);
}

PMIC_PROTOCOL gPmicProtocol = {
};

EFI_STATUS
EFIAPI
PmicDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  EFI_HANDLE  Handle = NULL;
  int rc;
  UINT8 revision = 0;

// Find the ssbi controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gSsbiProtocolGuid, NULL, (VOID **)&gSsbi);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallMultipleProtocolInterfaces(
  &Handle, &gPmicProtocolGuid, &gPmicProtocol, NULL);
  ASSERT_EFI_ERROR(Status);

  DEBUG((EFI_D_ERROR, "PmicDxe Initalize"));
  rc = pm8058_readb(PM8058_REG_REV, &revision);
	if (rc)
    DEBUG((EFI_D_ERROR, "%s: Failed on pm8058_readb for revision: rc=%d.\n",__func__, rc));

  DEBUG((EFI_D_ERROR, "%s: PMIC revision: %X\n", __func__, revision));
  for(;;){};

	return Status;
}