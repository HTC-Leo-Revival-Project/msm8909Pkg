#ifndef _ANDROID_SDDIR_H_
#define _ANDROID_SDDIR_H_

//GLOBAL VAR FOR ALL OF MENU APP
extern CHAR16 *SelectedDir;

//function definitions
EFI_STATUS DisplayAndSelectDirectory(CHAR16 **DirList, UINTN DirCount, CHAR16 **SelectedDir);
void SetAndroidSdDir(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);
EFI_STATUS ListDirectories(EFI_HANDLE *HandleBuffer, UINTN HandleCount, CHAR16 ***DirList, UINTN *DirCount);
#endif