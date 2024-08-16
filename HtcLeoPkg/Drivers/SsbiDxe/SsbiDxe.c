#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/Ssbi.h>

#include <Chipset/ssbi_msm7230.h>
#include <Chipset/iomap_msm7230.h>

void ssbi_test(void){
	//whatever
}

SSBI_PROTOCOL gSsbiProtocol = {
  ssbi_test
};

EFI_STATUS
EFIAPI
SsbiDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  EFI_HANDLE  Handle = NULL;

  Status = gBS->InstallMultipleProtocolInterfaces(
  &Handle, &gSsbiProtocolGuid, &gSsbiProtocol, NULL);
  ASSERT_EFI_ERROR(Status);


	return Status;
}