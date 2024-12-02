#include "../menu.h"
#include "BootimageTools.h"



BOOT_IMG_HDR Header;

BOOT_IMG_HDR* ParseBootImageHeader (IN EFI_FILE_PROTOCOL *File)
{
    EFI_STATUS Status;
    UINTN BufferSize = sizeof(BOOT_IMG_HDR);

    // Read the boot image header
    Status = File->Read(File, &BufferSize, &Header);
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Failed to read boot image header: %r\n", Status));
        return NULL;
    }

    // Verify the boot magic
    if (CompareMem(Header.magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0) {
        DEBUG((EFI_D_ERROR, "Invalid boot image magic\n"));
        return NULL;
    }

    return &Header;
}