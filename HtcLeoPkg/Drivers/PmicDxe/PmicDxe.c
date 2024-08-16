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

SSBI_PROTOCOL *gSsbi = NULL;

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

// Find the ssbi controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gSsbiProtocolGuid, NULL, (VOID **)&gSsbi);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallMultipleProtocolInterfaces(
  &Handle, &gPmicProtocolGuid, &gPmicProtocol, NULL);
  ASSERT_EFI_ERROR(Status);


	return Status;
}