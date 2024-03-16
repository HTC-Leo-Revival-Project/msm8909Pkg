#include "menu.h"
#include "BootApp.h"
#include <Chipset/iomap.h>
#include "DrawUI.h"

MenuEntry MenuOptions[MAX_OPTIONS_COUNT] = {0};

UINTN MenuOptionCount = 0;
UINTN SelectedIndex = 0;
EFI_SIMPLE_TEXT_OUTPUT_MODE InitialMode;


STATIC MENU_MSG_INFO mFastbootOptionTitle[] = {
    {{"START"},
     BIG_FACTOR,
     BGR_GREEN,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     RESTART},
    {{"Restart bootloader"},
     BIG_FACTOR,
     BGR_RED,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     FASTBOOT},
    {{"Recovery mode"},
     BIG_FACTOR,
     BGR_RED,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     RECOVER},
    {{"Power off"},
     BIG_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     POWEROFF},
#ifndef USER_BUILD_VARIANT
    {{"Boot to FFBM"},
     BIG_FACTOR,
     BGR_YELLOW,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     FFBM},
    {{"Boot to QMMI"},
     BIG_FACTOR,
     BGR_YELLOW,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     QMMI},
#endif
    /* Developer mode options */
    {{"Activate slot _a"},
     BIG_FACTOR,
     BGR_ORANGE,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     SET_ACTIVE_SLOT_A},
    {{"Activate slot _b"},
     BIG_FACTOR,
     BGR_ORANGE,
     BGR_BLACK,
     OPTION_ITEM,
     0,
     SET_ACTIVE_SLOT_B},
};

void
FillMenu()
{
  UINTN Index = 0;
  MenuOptions[Index++] = (MenuEntry){Index, L"Boot default", TRUE, &BootDefault};
  MenuOptions[Index++] = (MenuEntry){Index, L"Play Tetris", TRUE, &StartTetris};
  MenuOptions[Index++] = (MenuEntry){Index, L"EFI Shell", TRUE, &StartShell},
  MenuOptions[Index++] = (MenuEntry){Index, L"Reboot Menu", TRUE, &RebootMenu};
  MenuOptions[Index++] = (MenuEntry){Index, L"Exit", TRUE, &ExitMenu};
}

void PrepareConsole(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *Cout,
    OUT EFI_SIMPLE_TEXT_OUTPUT_MODE    *ModeToStore)
{
  EFI_STATUS Status;
  CopyMem(ModeToStore, Cout->Mode, sizeof(EFI_SIMPLE_TEXT_OUTPUT_MODE));

  Status = Cout->EnableCursor(Cout, FALSE);
  if (Status != EFI_UNSUPPORTED) { // workaround
    ASSERT_EFI_ERROR(Status);
  }

  Status = Cout->ClearScreen(Cout);
  ASSERT_EFI_ERROR(Status);
  Status = Cout->SetAttribute(Cout, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
  ASSERT_EFI_ERROR(Status);
  Status = Cout->SetCursorPosition(Cout, 0, 0);
  ASSERT_EFI_ERROR(Status);
}

void RestoreInitialConsoleMode(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *Cout,
    IN EFI_SIMPLE_TEXT_OUTPUT_MODE     *StoredMode)
{
  EFI_STATUS Status;

  Status = Cout->EnableCursor(Cout, StoredMode->CursorVisible);
  ASSERT_EFI_ERROR(Status);
  Status = Cout->SetCursorPosition(Cout, StoredMode->CursorColumn, StoredMode->CursorRow);
  ASSERT_EFI_ERROR(Status);
  Status = Cout->SetAttribute(Cout, StoredMode->Attribute);
  ASSERT_EFI_ERROR(Status);
  Status = Cout->ClearScreen(Cout);
  ASSERT_EFI_ERROR(Status);
}

UINTN GetActiveMenuEntryLength()
{
  UINTN MenuCount = 0;
  for (UINTN i = 0; i < sizeof(MenuOptions) / sizeof(MenuOptions[0]); i++) {
    if (MenuOptions[i].IsActive) {
      MenuCount++;
    }
  }
  return MenuCount;
}
extern EFI_STATUS DrawMenu (MENU_MSG_INFO *TargetMenu, UINT32 *pHeight);


/**
  Update the fastboot option item
  @param[in] OptionItem  The new fastboot option item
  @param[out] pLocation  The pointer of the location
  @retval EFI_SUCCESS	 The entry point is executed successfully.
  @retval other		 Some error occurs when executing this entry point.
 **/
EFI_STATUS
UpdateFastbootOptionItem (UINT32 OptionItem, UINT32 *pLocation)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 Location = 0;
  UINT32 Height = 0;
  MENU_MSG_INFO *FastbootLineInfo = NULL;

  FastbootLineInfo = AllocateZeroPool (sizeof (MENU_MSG_INFO));
  if (FastbootLineInfo == NULL) {
    DEBUG ((EFI_D_ERROR, "Failed to allocate zero pool.\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  SetMenuMsgInfo (FastbootLineInfo, "__________", COMMON_FACTOR,
                  mFastbootOptionTitle[OptionItem].FgColor,
                  mFastbootOptionTitle[OptionItem].BgColor, LINEATION, Location,
                  NOACTION);
  Status = DrawMenu (FastbootLineInfo, &Height);
  if (Status != EFI_SUCCESS)
    goto Exit;
  Location += Height;

  mFastbootOptionTitle[OptionItem].Location = Location;
  Status = DrawMenu (&mFastbootOptionTitle[OptionItem], &Height);
  if (Status != EFI_SUCCESS)
    goto Exit;
  Location += Height;

  FastbootLineInfo->Location = Location;
  Status = DrawMenu (FastbootLineInfo, &Height);
  if (Status != EFI_SUCCESS)
    goto Exit;
  Location += Height;

Exit:
  FreePool (FastbootLineInfo);
  FastbootLineInfo = NULL;

  if (pLocation != NULL)
    *pLocation = Location;

  return Status;
}

STATIC MENU_MSG_INFO mFastbootCommonMsgInfo[] = {
    {{"\nPress volume key to select, "
      "and press power key to confirm\n\n"},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"Fastboot mode\n\n"},
     COMMON_FACTOR,
     BGR_YELLOW,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"CURRENT_SLOT - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"\nPRODUCT_NAME - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"PRODUCT_MODEL - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"VARIANT - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"BOOTLOADER VERSION - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"BASEBAND VERSION - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"SERIAL NUMBER - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"HARDWARE REVISION - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"SECURE BOOT - "},
     COMMON_FACTOR,
     BGR_WHITE,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"\nDEVICE STATE - unlocked"},
     COMMON_FACTOR,
     BGR_RED,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"\nDEVICE STATE - locked"},
     COMMON_FACTOR,
     BGR_GREEN,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
    {{"\n"},
     COMMON_FACTOR,
     BGR_GREEN_DARK,
     BGR_BLACK,
     COMMON,
     0,
     NOACTION},
};




#define FASTBOOT_MSG_INDEX_HEADER                  0
#define FASTBOOT_MSG_INDEX_FASTBOOT                1
#define FASTBOOT_MSG_INDEX_CURRENT_SLOT            2
#define FASTBOOT_MSG_INDEX_PRODUCT_NAME            3
#define FASTBOOT_MSG_INDEX_PRODUCT_MODEL           4
#define FASTBOOT_MSG_INDEX_VARIANT                 5
#define FASTBOOT_MSG_INDEX_BOOTLOADER_VERSION      6
#define FASTBOOT_MSG_INDEX_BASEBAND_VERSION        7
#define FASTBOOT_MSG_INDEX_SERIAL_NUMBER           8
#define FASTBOOT_MSG_INDEX_HARDWARE_REVISION       9
#define FASTBOOT_MSG_INDEX_SECURE_BOOT            10
#define FASTBOOT_MSG_INDEX_DEVICE_STATE_UNLOCKED  11
#define FASTBOOT_MSG_INDEX_DEVICE_STATE_LOCKED    12
#define FASTBOOT_MSG_INDEX_MAINLINE               13
#define MAX_VERSION_LEN 64
#define OPTIONS_COUNT_DEVELOPER_MODE 2


STATIC CONST CHAR8 MAINLINE_LOGO_PMOS[][MAX_RSP_SIZE] = {
"         /\\\n",
"        /  \\\n",
"       /    \\\n",
"       \\__   \\\n",
"     /\\__ \\   \\\n",
"    /   /  \\  _\\\n",
"   /   /    \\/ __\n",
"  /   / ______/  \\\n",
" /    \\ \\         \\\n",
"/_____/ /__________\\\n",
};

/**
  Draw the fastboot menu
  @param[out] OptionMenuInfo  Fastboot option info
  @retval     EFI_SUCCESS     The entry point is executed successfully.
  @retval     other           Some error occurs when executing this entry point.
 **/
STATIC EFI_STATUS
FastbootMenuShowScreen (OPTION_MENU_INFO *OptionMenuInfo)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 Location = 0;
  UINT32 OptionItem = 0;
  UINT32 OptionItemCount = 0;
  UINT32 Height = 0;
  UINT32 i = 0;
  UINT32 j = 0;
  CHAR8 StrTemp[MAX_RSP_SIZE] = "";
  CHAR8 StrTemp1[MAX_RSP_SIZE] = "";
  CHAR8 VersionTemp[MAX_VERSION_LEN] = "";

  ZeroMem (&OptionMenuInfo->Info, sizeof (MENU_OPTION_ITEM_INFO));

  /* Update fastboot option title */
  OptionMenuInfo->Info.MsgInfo = mFastbootOptionTitle;
  for (i = 0; i < ARRAY_SIZE (mFastbootOptionTitle); i++) {
    OptionMenuInfo->Info.OptionItems[i] = i;
  }
  OptionItem =
      OptionMenuInfo->Info.OptionItems[OptionMenuInfo->Info.OptionIndex];
  Status = UpdateFastbootOptionItem (OptionItem, &Location);
  if (Status != EFI_SUCCESS)
    return Status;

  /* Update fastboot common message */
  for (i = 0; i < ARRAY_SIZE (mFastbootCommonMsgInfo); i++) {
    switch (i) {
    case FASTBOOT_MSG_INDEX_HEADER:
    case FASTBOOT_MSG_INDEX_FASTBOOT:
      break;
    case FASTBOOT_MSG_INDEX_CURRENT_SLOT:
      break;
    case FASTBOOT_MSG_INDEX_PRODUCT_NAME:
      /* Get product name */
      AsciiStrnCatS (mFastbootCommonMsgInfo[i].Msg,
        sizeof (mFastbootCommonMsgInfo[i].Msg), "HTC LEO",
        AsciiStrLen ("HTC LEO"));
      break;
    case FASTBOOT_MSG_INDEX_PRODUCT_MODEL:
      /* Get product model */
      AsciiStrnCatS (mFastbootCommonMsgInfo[i].Msg,
        sizeof (mFastbootCommonMsgInfo[i].Msg), "LEO",
        AsciiStrLen ("LEO"));
      break;
    case FASTBOOT_MSG_INDEX_VARIANT:
      /* Get variant value */
      break;
    case FASTBOOT_MSG_INDEX_BOOTLOADER_VERSION:
      /* Get bootloader version */
      break;
    case FASTBOOT_MSG_INDEX_BASEBAND_VERSION:
      /* Get baseband version */
      break;
    case FASTBOOT_MSG_INDEX_SERIAL_NUMBER:
      /* Get serial number */

      break;
    case FASTBOOT_MSG_INDEX_HARDWARE_REVISION:
      /* Get hardware revision */
      break;
    case FASTBOOT_MSG_INDEX_SECURE_BOOT:
      /* Get secure boot value */
      AsciiStrnCatS (
          mFastbootCommonMsgInfo[i].Msg, sizeof (mFastbootCommonMsgInfo[i].Msg),"no",AsciiStrLen ("no"));
      break;
    case FASTBOOT_MSG_INDEX_DEVICE_STATE_UNLOCKED:
      /* Get device status, only show when unlocked */
      break;
    case FASTBOOT_MSG_INDEX_DEVICE_STATE_LOCKED:
      /* Get device status, only show when locked */
        continue;
      break;
    case FASTBOOT_MSG_INDEX_MAINLINE:
      /* Print random mainline logo (right now only pmOS though) */

      for (j = 0; j < ARRAY_SIZE (MAINLINE_LOGO_PMOS); j++) {
        AsciiStrnCatS (
            mFastbootCommonMsgInfo[i].Msg, sizeof (mFastbootCommonMsgInfo[i].Msg),
            MAINLINE_LOGO_PMOS[j], sizeof (MAINLINE_LOGO_PMOS[j]));
      }
      break;
    }

    mFastbootCommonMsgInfo[i].Location = Location;
    Status = DrawMenu (&mFastbootCommonMsgInfo[i], &Height);
    if (Status != EFI_SUCCESS)
      return Status;
    Location += Height;
  }

  OptionMenuInfo->Info.MenuType = DISPLAY_MENU_FASTBOOT;

  OptionItemCount = ARRAY_SIZE (mFastbootOptionTitle);
    OptionItemCount -= OPTIONS_COUNT_DEVELOPER_MODE;
  OptionMenuInfo->Info.OptionNum = OptionItemCount;

  return Status;
}
STATIC OPTION_MENU_INFO gMenuInfo;
/* Draw the fastboot menu and start to detect the key's status */
VOID DisplayFastbootMenu (VOID)
{
  EFI_STATUS Status;
  OPTION_MENU_INFO *OptionMenuInfo;


    OptionMenuInfo = &gMenuInfo;
    DrawMenuInit ();
    OptionMenuInfo->LastMenuType = OptionMenuInfo->Info.MenuType;

    Status = FastbootMenuShowScreen (OptionMenuInfo);
    if (Status != EFI_SUCCESS) {
      DEBUG ((EFI_D_ERROR, "Unable to show fastboot menu on screen: %r\n",
              Status));
              Print("Unable to show fastboot menu on screen: %r\n",
              Status);
      return;
    }

   // MenuKeysDetectionInit (OptionMenuInfo);
    DEBUG ((EFI_D_VERBOSE, "Creating fastboot menu keys detect event\n"));
}


extern VOID DrawMenuInit (VOID);
void MainMenu(
    VOID)
{
  SelectedIndex     = 0;
  //EFI_STATUS Status = SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  //ASSERT_EFI_ERROR(Status);
  //DrawMenuInit();
  DisplayFastbootMenu();
  

  // FillMenu();
}

unsigned boot_reason = 0xFFFFFFFF;
unsigned android_reboot_reason = 0;
void get_boot_reason(void)
{
	// if(boot_reason==0xFFFFFFFF) {
		boot_reason = MmioRead32(MSM_SHARED_BASE+0xef244);
		// dprintf(INFO, "boot reason %x\n", boot_reason);
    DEBUG((EFI_D_ERROR, "boot reason %x\n", boot_reason));
    Print(L"boot reason %x\n", boot_reason);
		if(boot_reason!=2) {
			if(MmioRead32(0x2FFB0000)==(MmioRead32(0x2FFB0004)^0x004b4c63)) {
        DEBUG((EFI_D_ERROR, "CLK WAS DETECTED\n"));
			}
		}
	//}
	//return boot_reason;
}

void HandleKeyInput(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_INPUT_KEY key;
  EFI_STATUS    Status;

  Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);

  if (Status != EFI_NOT_READY) {
    ASSERT_EFI_ERROR(Status);

    switch (key.ScanCode) {
      case SCAN_HOME:
        // home button
        MainMenu();
        break;
      case SCAN_UP:
        // volume up button
        if (SelectedIndex == 0) {
          SelectedIndex = MenuOptionCount - 1;
        }
        else {
          SelectedIndex--;
        }
        break;
      case SCAN_DOWN:
        // volume down button
        if (SelectedIndex == MenuOptionCount - 1) {
          SelectedIndex = 0;
        }
        else {
          SelectedIndex++;
        }
        break;
      case SCAN_ESC:
        // power button
        break;
      default:
        switch (key.UnicodeChar) {
        case CHAR_CARRIAGE_RETURN:
          // dial button
          if (MenuOptions[SelectedIndex].Function != NULL) {
            MenuOptions[SelectedIndex].Function(ImageHandle, SystemTable);
          }
          break;
        case CHAR_TAB:
          // windows button
          DEBUG(
              (EFI_D_ERROR, "%d Menuentries are marked as active\n",
              GetActiveMenuEntryLength()));
          DEBUG((EFI_D_ERROR, "SelectedIndex is: %d\n", SelectedIndex));
          break;
        case CHAR_BACKSPACE:
          // back button
          break;
        default:
          break;
        }
        break;
    }
  }
}

// Start another app
void StartApp(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable, CHAR16 *Description)
{
  EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
  UINTN                         BootOptionCount;
  EFI_STATUS                    Status;
  
  EfiBootManagerRefreshAllBootOption();

  BootOptions =
      EfiBootManagerGetLoadOptions(&BootOptionCount, LoadOptionTypeBoot);
  ASSERT(BootOptionCount != -1);
  for (UINTN i = 0; i < BootOptionCount; i++) {
    if (StrCmp(Description, BootOptions[i].Description) == 0) {
      RestoreInitialConsoleMode(SystemTable->ConOut, &InitialMode);
      EfiBootManagerBoot(&BootOptions[i]);
    }
  }

  EfiBootManagerFreeLoadOptions(BootOptions, BootOptionCount);
}

void StartShell(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  StartApp(ImageHandle, SystemTable, SHELL_APP_TITLE);
}

void StartTetris(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  StartApp(ImageHandle, SystemTable, TETRIS_APP_TITLE);
}


void RebootMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  SelectedIndex     = 0;
  UINT8 Index = 0;
  EFI_STATUS Status = EFI_SUCCESS;
  
  Status = SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  ASSERT_EFI_ERROR(Status);
  MenuOptions[Index++] = (MenuEntry){Index, L"Reboot to CLK", TRUE, &get_boot_reason};
  MenuOptions[Index++] = (MenuEntry){Index, L"Reboot", TRUE, &ResetCold};
  MenuOptions[Index++] = (MenuEntry){Index, L"Shutdown", TRUE, &htcleo_shutdown};
  // Fill disabled options
  do {
    MenuOptions[Index++] = (MenuEntry){Index, L"", FALSE, &NullFunction};
  }while(Index < MAX_OPTIONS_COUNT);
}

void NullFunction()
{
  //Print(L"Feature not supported yet!");
  DEBUG((EFI_D_ERROR, "Feature not supported yet!"));
}

void ExitMenu(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status = SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  ASSERT_EFI_ERROR(Status);
  SystemTable->BootServices->Exit(ImageHandle, EFI_SUCCESS, 0, NULL);
}

void BootDefault(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status = EFI_SUCCESS;

  Status = DiscoverAndBootApp(
      ImageHandle, EFI_REMOVABLE_MEDIA_FILE_NAME_ARM, NULL);

  // We shouldn't reach here if the default file is present
  if(Status) {
    //Print(L"Booting default entry failed!");
    DEBUG((EFI_D_ERROR, "Booting default entry failed!"));
  }
}

EFI_STATUS EFIAPI
ShellAppMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  EFI_INPUT_KEY key;
  UINT32 Timeout = 400; //TODO: Get from pcd

  Print(L" Press Home within %d seconds to boot to menu\n", (Timeout / 100));
  Print(L" or back key to boot from ESP\n");

  do {
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);

    if (Status != EFI_NOT_READY) {
      ASSERT_EFI_ERROR(Status);

      switch (key.ScanCode) {
        case SCAN_HOME:
          // home button
          goto menu;
          break;
        case CHAR_BACKSPACE:
          goto boot_esp;
          break;
        default:
          break;
      }
    }
    // TODO: Use events?
    MicroSecondDelay(10000);
    Timeout--;
  }while(Timeout);

boot_esp:
  BootDefault(ImageHandle, SystemTable);

  // We should not get here if bootarm.efi is present, inform the user
  Print(L" Could not boot from ESP, loading menu\n");

menu:
  // Fill main menu
  // FillMenu();
  // DrawMenu();
  MainMenu();

  //PrepareConsole(SystemTable->ConOut, &InitialMode);

  // Loop for key input
  // while (TRUE) {
  //   
  //   HandleKeyInput(ImageHandle, SystemTable);
  // }

  // Restore initial console mode
 // RestoreInitialConsoleMode(SystemTable->ConOut, &InitialMode);

  return EFI_SUCCESS;
}