#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512

typedef struct {
    CHAR8 magic[BOOT_MAGIC_SIZE];
    UINT32 kernel_size;  /* size in bytes */
    UINT32 kernel_addr;  /* physical load addr */
    UINT32 ramdisk_size; /* size in bytes */
    UINT32 ramdisk_addr; /* physical load addr */
    UINT32 second_size;  /* size in bytes */
    UINT32 second_addr;  /* physical load addr */
    UINT32 tags_addr;    /* physical addr for kernel tags */
    UINT32 page_size;    /* flash page size we assume */
    UINT32 unused[2];    /* future expansion: should be 0 */
    CHAR8 name[BOOT_NAME_SIZE]; /* asciiz product name */
    CHAR8 cmdline[BOOT_ARGS_SIZE];
    UINT32 id[8]; /* timestamp / checksum / sha1 / etc */
} BOOT_IMG_HDR;

EFI_STATUS
EFIAPI
ParseBootImageHeader (
  IN EFI_FILE_PROTOCOL *File
  )
{
    EFI_STATUS Status;
    BOOT_IMG_HDR Header;
    UINTN BufferSize = sizeof(BOOT_IMG_HDR);

    // Read the boot image header
    Status = File->Read(File, &BufferSize, &Header);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to read boot image header: %r\n", Status);
        return Status;
    }

    // Verify the boot magic
    if (CompareMem(Header.magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0) {
        Print(L"Invalid boot image magic\n");
        return EFI_UNSUPPORTED;
    }

    // Print the header information
    Print(L"Boot image magic: %a\n", BOOT_MAGIC);
    Print(L"Kernel size: %u bytes\n", Header.kernel_size);
    Print(L"Kernel address: 0x%08x\n", Header.kernel_addr);
    Print(L"Ramdisk size: %u bytes\n", Header.ramdisk_size);
    Print(L"Ramdisk address: 0x%08x\n", Header.ramdisk_addr);
    Print(L"Second stage size: %u bytes\n", Header.second_size);
    Print(L"Second stage address: 0x%08x\n", Header.second_addr);
    Print(L"Page size: %u bytes\n", Header.page_size);
    Print(L"Cmdline: %a\n", Header.cmdline);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *File;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    CHAR16 *FileName = L"boot.img"; // Name of the boot image file

    // Locate the loaded image protocol
    Status = gBS->HandleProtocol(
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate loaded image protocol: %r\n", Status);
        return Status;
    }

    // Locate the file system that contains the boot image
    Status = gBS->HandleProtocol(
                  LoadedImage->DeviceHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&FileSystem);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate simple file system protocol: %r\n", Status);
        return Status;
    }

    // Open the root directory
    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open root directory: %r\n", Status);
        return Status;
    }

    // Open the boot image file
    Status = Root->Open(Root, &File, FileName, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open boot image file: %r\n", Status);
        return Status;
    }

    // Parse the boot image header
    Status = ParseBootImageHeader(File);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to parse boot image header: %r\n", Status);
    }

    // Close the file and root
    File->Close(File);
    Root->Close(Root);

    return Status;
}
