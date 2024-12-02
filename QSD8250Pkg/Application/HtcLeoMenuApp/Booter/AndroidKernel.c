#include "../menu.h"
#include "AndroidKernel.h"
#include <Library/FrameBufferConfigLib.h>
#include "LinuxShim.h"
#include "AndroidSDDir.h"
#include "LinuxHeader.h"
#include "BootimageTools.h"

#define BASE_ADDR FixedPcdGet32(PcdSystemMemoryBase)

VOID *KernelLoadAddress = (VOID *)(BASE_ADDR + KERNEL_OFFSET);
VOID *RamdiskLoadAddress = (VOID *)(BASE_ADDR + RAMDISK_OFFSET);
UINTN *AtagsAddress = (unsigned *)(BASE_ADDR + TAGS_OFFSET);
UINTN MachType = 0x9DC;
EFI_FILE_PROTOCOL *BootFile;
//UINTN MachType = FixedPcdGet32(PcdMachType);

unsigned* target_atag_mem(unsigned* ptr)
{
	//add atag to notify nand boot
	*ptr++ = 4;
	*ptr++ = 0x4C47414D; 	// NAND atag (MAGL :))
	*ptr++ = 0x004b4c63; 	// cLK signature
	*ptr++ = 13;			// cLK version number

	return ptr;
}

VOID
CallExitBS(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status;
    
    UINTN MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    UINTN LocalMapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    MemoryMap = NULL;
    MemoryMapSize = 0;
    
    /* Get the memory map */
    do {  
        Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &LocalMapKey, &DescriptorSize,&DescriptorVersion);
        if (Status == EFI_BUFFER_TOO_SMALL){
            MemoryMap = AllocatePool(MemoryMapSize + 1);
            Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &LocalMapKey, &DescriptorSize,&DescriptorVersion);      
        } else {
            /* Status is likely success - let the while() statement check success */
        }
    
    } while (Status != EFI_SUCCESS);

    DEBUG((EFI_D_INFO, "Exit BS\n"));
    gBS->ExitBootServices(ImageHandle, LocalMapKey);
}


VOID AsciiStrToUnicodeStr(CONST CHAR8 *Source, CHAR16 *Destination) {
    while (*Source != '\0') {
        *(Destination++) = (CHAR16)*(Source++);
    }
    *Destination = '\0'; // Null-terminate the Unicode string
}

/* Loads a raw image */
VOID
BootImage(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable,
    void *Kernel
)
{
	EFI_STATUS Status;
	void (*Entry)(unsigned,unsigned,unsigned*) = Kernel;

	Print(L"booting image @ %p\n", Kernel);

    CallExitBS(ImageHandle, SystemTable);

	//we are ready to boot the freshly loaded kernel
	PrepareForLinux();
	Entry(0, 0, 0);

    DEBUG((EFI_D_ERROR, "Failed to boot image, jump did not happen were still in uefiland \n\n"));
	//deadlock the platform, this is not recoverable
	for(;;);
}

/* Loads a linux image */
VOID
BootLinux(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable,
    void *kernel, 
    unsigned *tags, 
	const char *cmdline, 
    unsigned MachType,
	void *ramdisk, 
    unsigned ramdisk_size)
{
	EFI_STATUS Status;
	unsigned *ptr = tags;
	//unsigned pcount = 0;
	void (*entry)(unsigned,unsigned,unsigned*) = kernel;
	//struct ptable *ptable;
	int cmdline_len = 0;
	int have_cmdline = 0;

	/* CORE */
	*ptr++ = 2;
	*ptr++ = 0x54410001;

	if (ramdisk_size) {
		*ptr++ = 4;
		*ptr++ = 0x54420005;
		*ptr++ = (unsigned)ramdisk;
		*ptr++ = ramdisk_size;
	}

	ptr = target_atag_mem(ptr);
    /* ToDo: add the needed nand stuff ????*/
	// if (!target_is_emmc_boot()) {
	// 	/* Skip NAND partition ATAGS for eMMC boot */
	// 	if ((ptable = flash_get_ptable()) && (ptable->count != 0)) {
	// 		int i;
	// 		for(i=0; i < ptable->count; i++) {
	// 			struct ptentry *ptn;
	// 			ptn =  ptable_get(ptable, i);
	// 			if (ptn->type == TYPE_APPS_PARTITION)
	// 				pcount++;
	// 		}
	// 		*ptr++ = 2 + (pcount * (sizeof(struct atag_ptbl_entry) /
	// 					       sizeof(unsigned)));
	// 		*ptr++ = 0x4d534d70;
	// 		for (i = 0; i < ptable->count; ++i)
	// 			ptentry_to_tag(&ptr, ptable_get(ptable, i));
	// 	}
	// }

	if (cmdline && cmdline[0]) {
		cmdline_len = AsciiStrLen(cmdline);
		have_cmdline = 1;
	}
	if (cmdline_len > 0) {
		const char *src;
		char *dst;
		unsigned n;
		/* include terminating 0 and round up to a word multiple */
		n = (cmdline_len + 4) & (~3);
		*ptr++ = (n / 4) + 2;
		*ptr++ = 0x54410009;
		dst = (char *)ptr;
		if (have_cmdline) {
			src = cmdline;
			while ((*dst++ = *src++));
		}
		ptr += (n / 4);
	}

	/* END */
	*ptr++ = 0;
	*ptr++ = 0;
	Print(L"booting linux @ %p, ramdisk @ %p (%d)\n",kernel, ramdisk, ramdisk_size);
	    if (cmdline) {
        CHAR16 CmdlineUnicode[256];
        AsciiStrToUnicodeStr(cmdline, CmdlineUnicode);
        Print(L"cmdline: %s\n", CmdlineUnicode);
    }

    CallExitBS(ImageHandle, SystemTable);

	//we are ready to boot the freshly loaded kernel
	PrepareForLinux();
	entry(0, MachType, tags);

    DEBUG((EFI_D_ERROR, "Failed to boot Linux, jump did not happen were still in uefiland \n\n"));
	//deadlock the platform this is not recoverable
	for(;;);
}

EFI_STATUS
EFIAPI
LoadFileFromSDCard(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable,
    IN CHAR16 *KernelFileName
)
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
    EFI_FILE_PROTOCOL *Root;

    // Locate the handle for the SD card
    EFI_HANDLE *Handles;
    UINTN HandleCount;
    Status = SystemTable->BootServices->LocateHandleBuffer(
                ByProtocol,
                &gEfiSimpleFileSystemProtocolGuid,
                NULL,
                &HandleCount,
                &Handles
             );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate handles for SimpleFileSystem: %r\n", Status);
        return Status;
    }

    // Assume the SD card is the first handle found
    Status = SystemTable->BootServices->HandleProtocol(
                Handles[0],
                &gEfiSimpleFileSystemProtocolGuid,
                (VOID **)&SimpleFileSystem
             );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to handle SimpleFileSystem protocol: %r\n", Status);
        return Status;
    }

    // Open the root directory of the SD card
    Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open volume: %r\n", Status);
        return Status;
    }

    // Open the kernel file specified by KernelFileName
    Status = Root->Open(
                Root,
                BootFile,
                KernelFileName,
                EFI_FILE_MODE_READ,
                0
             );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel file %s: %r\n", KernelFileName, Status);
        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ReadBootFileIntoMemory(
    IN EFI_SYSTEM_TABLE *SystemTable,
    IN VOID *LoadAddress, // New parameter for custom load address
    OUT VOID **KernelBuffer,
    OUT UINTN *KernelSize
)
{
    EFI_STATUS Status;
    UINTN BufferSize;
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO);
    VOID *Buffer = NULL;

    // Get the file size
    Status = BootFile->GetInfo(
                BootFile,
                &gEfiFileInfoGuid,
                &FileInfoSize,
                NULL
             );
    if (Status != EFI_BUFFER_TOO_SMALL) {
        Print(L"Failed to get file info size: %r\n", Status);
        return Status;
    }

    FileInfo = AllocatePool(FileInfoSize);
    if (FileInfo == NULL) {
        Print(L"Failed to allocate memory for file info\n");
        return EFI_OUT_OF_RESOURCES;
    }

    Status = BootFile->GetInfo(
                BootFile,
                &gEfiFileInfoGuid,
                &FileInfoSize,
                FileInfo
             );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get file info: %r\n", Status);
        FreePool(FileInfo);
        return Status;
    }

    // Allocate buffer for the kernel at the specified load address
    BufferSize = (UINTN)FileInfo->FileSize;
    Buffer = LoadAddress; // Use the specified LoadAddress
    if (Buffer == NULL) {
        Print(L"Failed to allocate memory at load address\n");
        FreePool(FileInfo);
        return EFI_OUT_OF_RESOURCES;
    }

    // Read the kernel file into the buffer
    Status = BootFile->Read(
                BootFile,
                &BufferSize,
                Buffer
             );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to read kernel file: %r\n", Status);
        FreePool(FileInfo);
        return Status;
    }

    // Clean up and return the buffer
    FreePool(FileInfo);
    BootFile->Close(BootFile);

    *KernelBuffer = Buffer;
    *KernelSize = BufferSize;

    return EFI_SUCCESS;
}


void BootAndroidKernel(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    VOID *KernelBuffer;
    UINTN KernelSize;
    VOID *RamdiskBuffer;
    UINTN RamdiskSize;
    CHAR16 KernelPath[256];
    CHAR16 RamdiskPath[256];

    const char *cmdline;// = "androidboot.hardware=htcleo androidboot.selinux=permissive androidboot.configfs=false";
    
    CHAR8 *AllocatedCmdline = NULL;
    UINTN CmdlineSize;

    Status = SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    ASSERT_EFI_ERROR(Status);

    //toDo: add support for boot.img
    if (!FallBack) {
        // Build the paths based on SelectedDir
        UnicodeSPrint(KernelPath, sizeof(KernelPath), L"\\%s\\zImage", SelectedDir);
        UnicodeSPrint(RamdiskPath, sizeof(RamdiskPath), L"\\%s\\initrd.img", SelectedDir);
    } else {
        // Use default paths
        StrCpyS(KernelPath, sizeof(KernelPath) / sizeof(CHAR16), L"\\zImage");
        StrCpyS(RamdiskPath, sizeof(RamdiskPath) / sizeof(CHAR16), L"\\initrd.img");
    }
    // Print the path that was generated
    DEBUG((EFI_D_INFO, "BootDir is %s\n", KernelPath, Status));

    Status = LoadFileFromSDCard(ImageHandle, SystemTable, KernelPath);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load Boot File from path %s: %r\n", KernelPath, Status);
    } else {
        Print(L"Boot File loaded successfully from path %s\n", KernelPath);
        
        BOOT_IMG_HDR* Header = ParseBootImageHeader(BootFile);
        //since we never load the file into memory here the handle would never be closed so close it manually
        BootFile->Close(BootFile);
        if (Header){
            DEBUG((EFI_D_INFO, "Image is an Android Bootimage!\n"));
            //take needed values here
            cmdline = Header->cmdline;
            KernelSize = Header->kernel_size;
            KernelLoadAddress = (VOID *)Header->kernel_addr;
            RamdiskLoadAddress = (VOID *)Header->ramdisk_addr;


            //somehow load only the kernel and ramdisk here to the correct adresses read from bootimage and then boot
            //boot the kernel
            BootImage(ImageHandle, SystemTable, KernelBuffer);
        }else {
            //load the image into the correct adress

            Status = ReadBootFileIntoMemory(SystemTable, KernelLoadAddress, &KernelBuffer, &KernelSize);
            if (Status != EFI_SUCCESS){
                DEBUG((EFI_D_INFO, "Image loading into address %p\n", KernelLoadAddress));
                goto cleanup;
            }else {
                DEBUG((EFI_D_INFO, "Image loaded into memory adress %p. Size: %d bytes\n", KernelBuffer, KernelSize));
            }
            DEBUG((EFI_D_INFO, "Image is not an Android Bootimage, testing for zImage\n"));
        /* Check if the image is a linux kernel 
         * source: https://github.com/ARM-software/u-boot/blob/master/arch/arm/lib/zimage.c
         */
        struct arm_z_header *zi = (struct arm_z_header *)KernelBuffer;
        if (zi->zi_magic != LINUX_ARM_ZIMAGE_MAGIC) {
            DEBUG((EFI_D_INFO, "Bad Linux ARM zImage magic, booting only the image\n"));

            // Reconfigure the FB back to RGB565
            ReconfigFb(RGB565_BPP);
            
            // Boot the image
            BootImage(ImageHandle, SystemTable, KernelBuffer);

            // Clean up and loop if booting failed
            goto cleanup;
        }

        Status = LoadFileFromSDCard(ImageHandle, SystemTable, RamdiskPath);
        if (EFI_ERROR(Status)) {
            Print(L"Failed to load ramdisk from path %s: %r\n", RamdiskPath, Status);
        } else {
            Print(L"Ramdisk opened successfully from path %s\n", RamdiskPath);
        }

        Status = ReadBootFileIntoMemory(SystemTable, RamdiskLoadAddress, &RamdiskBuffer, &RamdiskSize);
        if (Status != EFI_SUCCESS){
            DEBUG((EFI_D_INFO, "Ramdisk loading into address %p failed\n", KernelLoadAddress));
            goto cleanup;
        }else {
            DEBUG((EFI_D_INFO, "Ramdisk loaded into memory adress %p. Size: %d bytes\n", KernelBuffer, KernelSize));
            }
        // Allocate memory for cmdline
        CmdlineSize = AsciiStrLen(cmdline) + 1; // +1 for the null terminator
        Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, CmdlineSize, (void **)&AllocatedCmdline);
        if (EFI_ERROR(Status)) {
            Print(L"Failed to allocate memory for cmdline: %r\n", Status);
        } else {
            // Copy cmdline to allocated memory
            AsciiStrCpyS(AllocatedCmdline, CmdlineSize, cmdline);

            // Reconfigure the FB back to RGB565
            ReconfigFb(RGB565_BPP);
            
            // Boot Linux with the allocated cmdline
            BootLinux(ImageHandle, SystemTable, KernelBuffer, AtagsAddress, 
                    AllocatedCmdline, MachType, RamdiskBuffer, RamdiskSize);
        }
            }
        cleanup:
            // Clean up allocated cmdline
            SystemTable->BootServices->FreePool(AllocatedCmdline);
        
        // Clean up the kernel buffer when no longer needed, but we should never reach here essentially halt the platform
        FreePool(KernelBuffer);
        Print(L"Booting Linux Failed, unknown error occurred");
        for (;;) ;
    }
}



