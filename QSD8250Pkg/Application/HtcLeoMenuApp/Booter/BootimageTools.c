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

EFI_STATUS LoadBootImageParts(
    BOOT_IMG_HDR *Header,
    EFI_FILE_PROTOCOL *BootImageFile,
    VOID **KernelBuffer,
    VOID **RamdiskBuffer
) {
    EFI_STATUS Status;
    UINTN PageSize = Header->page_size;
    UINTN Offset;
    UINTN ReadSize;

    // Validate the output pointers
    if (KernelBuffer == NULL || RamdiskBuffer == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    // Load kernel directly into specified address
    if (Header->kernel_size > 0) {
        Offset = PageSize;  // Kernel starts after the first page
        Status = BootImageFile->SetPosition(BootImageFile, Offset);
        if (EFI_ERROR(Status)) {
            DEBUG((EFI_D_ERROR, "Failed to set file position for kernel: %r\n", Status));
            return Status;
        }

        ReadSize = Header->kernel_size;
        Status = BootImageFile->Read(BootImageFile, &ReadSize, (VOID *)(UINTN)Header->kernel_addr);
        if (EFI_ERROR(Status) || ReadSize != Header->kernel_size) {
            DEBUG((EFI_D_ERROR, "Failed to read kernel: %r\n", Status));
            return Status;
        }

        // Return the kernel address to the caller
        *KernelBuffer = (VOID *)(UINTN)Header->kernel_addr;
        DEBUG((EFI_D_ERROR, "Kernel loaded to %p, size: %u bytes\n", *KernelBuffer, Header->kernel_size));
    } else {
        DEBUG((EFI_D_ERROR, "Kernel size is zero\n"));
        return EFI_INVALID_PARAMETER;
    }

    // Load ramdisk directly into specified address
    if (Header->ramdisk_size > 0) {
        Offset = PageSize + ALIGN_TO_PAGE(Header->kernel_size, PageSize);
        Status = BootImageFile->SetPosition(BootImageFile, Offset);
        if (EFI_ERROR(Status)) {
            DEBUG((EFI_D_ERROR, "Failed to set file position for ramdisk: %r\n", Status));
            return Status;
        }

        ReadSize = Header->ramdisk_size;
        Status = BootImageFile->Read(BootImageFile, &ReadSize, (VOID *)(UINTN)Header->ramdisk_addr);
        if (EFI_ERROR(Status) || ReadSize != Header->ramdisk_size) {
            DEBUG((EFI_D_ERROR, "Failed to read ramdisk: %r\n", Status));
            return Status;
        }

        // Return the ramdisk address to the caller
        *RamdiskBuffer = (VOID *)(UINTN)Header->ramdisk_addr;
        DEBUG((EFI_D_ERROR, "Ramdisk loaded to %p, size: %u bytes\n", *RamdiskBuffer, Header->ramdisk_size));
    }

    // Load second stage directly into specified address (if applicable)
    if (Header->second_size > 0) {
        Offset = PageSize + ALIGN_TO_PAGE(Header->kernel_size, PageSize) + ALIGN_TO_PAGE(Header->ramdisk_size, PageSize);
        Status = BootImageFile->SetPosition(BootImageFile, Offset);
        if (EFI_ERROR(Status)) {
            DEBUG((EFI_D_ERROR, "Failed to set file position for second stage: %r\n", Status));
            return EFI_SUCCESS;  // Graceful failure for second stage
        }

        ReadSize = Header->second_size;
        Status = BootImageFile->Read(BootImageFile, &ReadSize, (VOID *)(UINTN)Header->second_addr);
        if (EFI_ERROR(Status) || ReadSize != Header->second_size) {
            DEBUG((EFI_D_ERROR, "Failed to read second stage: %r\n", Status));
            return EFI_SUCCESS;  // Graceful failure for second stage
        }

        DEBUG((EFI_D_ERROR, "Second stage loaded to %p, size: %u bytes\n", (VOID *)(UINTN)Header->second_addr, Header->second_size));
    }

    return EFI_SUCCESS;
}