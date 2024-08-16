#include "commonvars.h"

SSBI_PROTOCOL *gSsbi = NULL;

UINT8 revision = 0;

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

  //clear the framebuffer so we can read what is happening

  ZeroMem((void*)0x2fd00000, 0x000C0000);

// Find the ssbi controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gSsbiProtocolGuid, NULL, (VOID **)&gSsbi);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallMultipleProtocolInterfaces(
  &Handle, &gPmicProtocolGuid, &gPmicProtocol, NULL);
  ASSERT_EFI_ERROR(Status);

  DEBUG((EFI_D_ERROR, "PmicDxe Initalize \n"));
  rc = pm8058_readb(PM8058_REG_REV, &revision);
	if (rc)
    DEBUG((EFI_D_ERROR, "%s: Failed on pm8058_readb for revision: rc=%d.\n","PmicDxeInitialize", rc));

  DEBUG((EFI_D_ERROR, "%s: PMIC revision: %X\n", __func__, revision));

	rc = pm8058_hard_reset_config(SHUTDOWN_ON_HARD_RESET);
	if (rc < 0){
    DEBUG((EFI_D_ERROR,"%s: failed to config shutdown on hard reset: %d\n","PmicDxeInitialize", rc));
  }







  for(;;){};

	return Status;
}