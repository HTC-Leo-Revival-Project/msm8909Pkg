#include "../menu.h"
#include "AndroidKernel.h"
#include <Library/FrameBufferConfigLib.h>

void htcleo_prepare_for_linux(void)
{
	// Martijn Stolk's code so kernel will not crash. aux control register
	__asm__ volatile("MRC p15, 0, r0, c1, c0, 1\n"
					 "BIC r0, r0, #0x40\n"
					 "BIC r0, r0, #0x200000\n"
					 "MCR p15, 0, r0, c1, c0, 1");

	// Disable VFP
	 __asm__ volatile("MOV R0, #0\n"
	 				 "FMXR FPEXC, r0");

    // disable mmu
	__asm__ volatile("MRC p15, 0, r0, c1, c0, 0\n"
					 "BIC r0, r0, #(1<<0)\n"
					 "MCR p15, 0, r0, c1, c0, 0\n"
					 "ISB");
	
	// Invalidate the UTLB
	__asm__ volatile("MOV r0, #0\n"
					 "MCR p15, 0, r0, c8, c7, 0");

	// Clean and invalidate cache - Ensure pipeline flush
	__asm__ volatile("MOV R0, #0\n"
					 "DSB\n"
					 "ISB");

	__asm__ volatile("BX LR");
}

unsigned* target_atag_mem(unsigned* ptr)
{

#if 0
	//MEM TAG
	*ptr++ = 4;
	*ptr++ = 0x54410002;
	//*ptr++ = 0x1e400000; //mem size from haret
	//*ptr++ = 0x1E7C0000; //mem size from kernel config
	*ptr++ = 0x1CFC0000; //mem size from kernel config with bravo dsp
	*ptr++ = 0x11800000; //mem base
#endif

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
    /* Get the memory map */
    UINTN MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    UINTN LocalMapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    MemoryMap = NULL;
    MemoryMapSize = 0;
    
	
    do {  
        Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &LocalMapKey, &DescriptorSize,&DescriptorVersion);
        if (Status == EFI_BUFFER_TOO_SMALL){
            MemoryMap = AllocatePool(MemoryMapSize + 1);
            Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &LocalMapKey, &DescriptorSize,&DescriptorVersion);      
        } else {
            /* Status is likely success - let the while() statement check success */
        }
        DEBUG((EFI_D_ERROR, "Memory loop iteration, status: %r\n", Status));
    
    } while (Status != EFI_SUCCESS);

    DEBUG((EFI_D_ERROR, "Exit BS\n"));
    gBS->ExitBootServices(ImageHandle, LocalMapKey);
}


VOID AsciiStrToUnicodeStr(CONST CHAR8 *Source, CHAR16 *Destination) {
    while (*Source != '\0') {
        *(Destination++) = (CHAR16)*(Source++);
    }
    *Destination = '\0'; // Null-terminate the Unicode string
}

void boot_linux(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,void *kernel, unsigned *tags, 
		const char *cmdline, unsigned machtype,
		void *ramdisk, unsigned ramdisk_size)
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
        /*J0SH1X: leo is never emmc boot, even tho sdsupport in clk would be cool*/
		// if (target_is_emmc_boot()) {
		// 	src = emmc_cmdline;
		// 	if (have_cmdline) --dst;
		// 	have_cmdline = 1;
		// 	while ((*dst++ = *src++));
		// }
        /*J0SH1X: we dont need clks charging since menu already handles charging itself*/
		// if (pause_at_bootup) {
		// 	src = battchg_pause;
		// 	if (have_cmdline) --dst;
		// 	while ((*dst++ = *src++));
		// }
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

    /*ToDo: add uefi version of this
	 arch_disable_cache(UCACHE);
	 arch_disable_mmu();
    */

    /*replace with crappy fade effect code i guess*/
// #if DISPLAY_SPLASH_SCREEN
// 	display_shutdown();
// #endif
    CallExitBS(ImageHandle, SystemTable);

	//we are ready to boot the freshly loaded kernel
	//DEBUG((EFI_D_INFO, "Preparing... \n"));
	//htcleo_prepare_for_linux();
	//DEBUG((EFI_D_INFO, "Jumping to kernel\n"));
	entry(0, machtype, tags);

    DEBUG((EFI_D_ERROR, "Failed to boot Linux, jump did not happen were still in uefiland \n\n"));
	//deadlock the platform this is not recoverable
	for(;;);
}

EFI_STATUS
EFIAPI
LoadFileFromSDCard(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable,
    IN CHAR16 *KernelFileName,
    IN VOID *LoadAddress, // New parameter for custom load address
    OUT VOID **KernelBuffer,
    OUT UINTN *KernelSize
)
{
    EFI_STATUS Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
    EFI_FILE_PROTOCOL *Root;
    EFI_FILE_PROTOCOL *KernelFile;
    UINTN BufferSize;
    VOID *Buffer = NULL;

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
                &KernelFile,
                KernelFileName,
                EFI_FILE_MODE_READ,
                0
             );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel file %s: %r\n", KernelFileName, Status);
        return Status;
    }

    // Get the file size
    EFI_FILE_INFO *FileInfo;
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO);
    Status = KernelFile->GetInfo(
                KernelFile,
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

    Status = KernelFile->GetInfo(
                KernelFile,
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
    Status = KernelFile->Read(
                KernelFile,
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
    KernelFile->Close(KernelFile);

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
    CHAR16 *KernelPath = L"\\zImage";
    CHAR16 *RamdiskPath = L"\\initrd.img";

    UINTN BaseAddr = FixedPcdGet32(PcdSystemMemoryBase);
    VOID *KernelLoadAddress = (VOID *)(BaseAddr + KERNEL_OFFSET);
    VOID *RamdiskLoadAddress = (VOID *)(BaseAddr + RAMDISK_OFFSET);
    unsigned *tags_address = (unsigned *)(BaseAddr + TAGS_OFFSET);
    const char *cmdline = "androidboot.hardware=htcleo androidboot.selinux=permissive androidboot.configfs=false";
    unsigned machtype = 0x9dc;
    
    CHAR8 *AllocatedCmdline = NULL;
    UINTN CmdlineSize;

    Status = SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    ASSERT_EFI_ERROR(Status);

    Status = LoadFileFromSDCard(ImageHandle, SystemTable, KernelPath, KernelLoadAddress, &KernelBuffer, &KernelSize);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to load kernel: %r\n", Status);
    } else {
        Print(L"Kernel loaded successfully at address %p. Size: %d bytes\n", KernelBuffer, KernelSize);
        
        Status = LoadFileFromSDCard(ImageHandle, SystemTable, RamdiskPath, RamdiskLoadAddress, &RamdiskBuffer, &RamdiskSize);
        if (EFI_ERROR(Status)) {
            Print(L"Failed to load ramdisk: %r\n", Status);
        } else {
            Print(L"Ramdisk loaded successfully at address %p. Size: %d bytes\n", RamdiskLoadAddress, RamdiskSize);
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
            boot_linux(ImageHandle, SystemTable, KernelBuffer, tags_address, AllocatedCmdline, machtype, RamdiskBuffer, RamdiskSize);
            
            // Clean up allocated cmdline
            SystemTable->BootServices->FreePool(AllocatedCmdline);
        }
        
        // Clean up the kernel buffer when no longer needed, but we should never reach here essentially halt the platform
        FreePool(KernelBuffer);
        Print(L"Booting Linux Failed, unknown error occurred");
        for (;;) ;
    }
}


