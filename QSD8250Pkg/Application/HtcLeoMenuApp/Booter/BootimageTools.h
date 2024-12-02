#ifndef _BOOTIMAGE_TOOLS_H_
#define _BOOTIMAGE_TOOLS_H_


#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512

#define ALIGN_TO_PAGE(size, page_size) (((size) + (page_size) - 1) / (page_size)) * (page_size)

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

BOOT_IMG_HDR* ParseBootImageHeader (IN EFI_FILE_PROTOCOL *File);
EFI_STATUS LoadBootImageParts(
    BOOT_IMG_HDR *Header,
    EFI_FILE_PROTOCOL *BootImageFile,
    VOID **KernelBuffer,
    VOID **RamdiskBuffer
);

#endif