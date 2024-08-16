#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/Pmic.h>

PMIC_PROTOCOL gPmicProtocol = {
};

extern UINTN EFIAPI MicroSecondDelay (IN      UINTN                     MicroSeconds);

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
  // Status = gBS->LocateProtocol (&gSsbiProtocolGuid, NULL, (VOID **)&gSsbi);
  // ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallMultipleProtocolInterfaces(
  &Handle, &gPmicProtocolGuid, &gPmicProtocol, NULL);
  ASSERT_EFI_ERROR(Status);

  DEBUG((EFI_D_ERROR, "PmicDxe Initalize \n"));


	return Status;
}