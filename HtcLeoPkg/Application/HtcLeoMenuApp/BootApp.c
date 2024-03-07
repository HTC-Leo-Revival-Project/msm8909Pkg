/** @file
  Support for loading app by path.

Copyright (c) 2011 - 2016, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2018 - 2019, Bingxing Wang. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


**/

#include "menu.h"
#include "BootApp.h"

EFI_STATUS EFIAPI DiscoverAndBootApp(
    IN EFI_HANDLE ImageHandle, CHAR16 *AppPath, CHAR16 *FallbackPath)
{
  EFI_HANDLE *               FileSystemHandles;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedAppImage;
  EFI_HANDLE                 LoadedAppHandle;
  UINTN                      NumberFileSystemHandles;
  EFI_STATUS                 Status;
  EFI_DEVICE_PATH_PROTOCOL * FilePath = NULL;

  Status = gBS->LocateHandleBuffer(
      ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL,
      &NumberFileSystemHandles, &FileSystemHandles);

  for (UINTN Handle = 0; Handle < NumberFileSystemHandles; Handle++) {
    FilePath = FileDevicePath(FileSystemHandles[Handle], AppPath);
    Status =
        gBS->LoadImage(TRUE, ImageHandle, FilePath, NULL, 0, &LoadedAppHandle);

    if (EFI_ERROR(Status) && FallbackPath != NULL) {
      FilePath = FileDevicePath(FileSystemHandles[Handle], FallbackPath);
      Status   = gBS->LoadImage(
          TRUE, ImageHandle, FilePath, NULL, 0, &LoadedAppHandle);
    }

    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = gBS->HandleProtocol(
        LoadedAppHandle, &gEfiLoadedImageProtocolGuid, (VOID *)&LoadedAppImage);

    if (!EFI_ERROR(Status)) {
      Status = gBS->StartImage(LoadedAppHandle, NULL, NULL);
      return Status;
    }
  }

  return Status;
}