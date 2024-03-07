#include <Uefi.h>
#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/BootAppLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/ArmLib.h>

#include <Library/HtcLeoPlatformResetLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/DevicePathLib.h>
#include <Library/TimerLib.h>

#include <Protocol/LoadedImage.h>
#include <Resources/FbColor.h>
#include <Chipset/timer.h>

#ifndef _MAIN_MENU_H_
#define _MAIN_MENU_H_

typedef struct {
  UINT8   Index;
  CHAR16 *Name;
  BOOLEAN IsActive;
  void (*Function)();
} MenuEntry;

#define MAX_OPTIONS_COUNT 12

void RebootMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void ExitMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void DrawMenu(IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConsoleOut);
void ReturnToMainMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
UINTN GetActiveMenuEntryLength();

void PrepareConsole(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *Cout,
    OUT EFI_SIMPLE_TEXT_OUTPUT_MODE    *ModeToStore);
void RestoreInitialConsoleMode(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *Cout,
    IN EFI_SIMPLE_TEXT_OUTPUT_MODE     *StoredMode);
void HandleKeyInput(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);

void NullFunction();
void StartTetris(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
void StartShell(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);

#endif