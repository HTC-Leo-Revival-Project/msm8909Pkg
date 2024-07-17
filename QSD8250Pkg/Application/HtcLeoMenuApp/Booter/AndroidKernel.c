#include "../menu.h"
#include <Chipset/interrupts.h>

static inline void htcleo_boot_s() {
    asm volatile (
        // Save registers
        "MOV r9, r0\n"

        // Cotulla's code so kernel will not crash. aux control register
        // Found more info here: http://www.spinics.net/lists/linux-arm-msm/msg00492.html
        // It looks it is Martijn Stolk's code
        "MRC p15, 0, r0, c1, c0, 1\n"
        "BIC r0, r0, #0x40\n"               // (1<<6)  IBE (0 = executes the CP15 Invalidate All and Invalidate by MVA instructions as a NOP instruction, reset value)
        "BIC r0, r0, #0x200000\n"           // (1<<21) undocumented bit
        "MCR p15, 0, r0, c1, c0, 1\n"

        // Disable VFP
        "MOV r0, #0\n"
        "FMXR FPEXC, r0\n"

        // ICIALL to invalidate entire I-Cache
        "MCR p15, 0, r0, c7, c5, 0\n"       // ICIALLU

        // Disable dcache and i cache
        "MRC p15, 0, r0, c1, c0, 0\n"
        "BIC r0, r0, #(1<<0)\n"             // disable mmu (already disabled)
        "BIC r0, r0, #(1<<2)\n"             // disable data cache
        "BIC r0, r0, #(1<<12)\n"            // disable instruction cache
        "MCR p15, 0, r0, c1, c0, 0\n"
        "ISB\n"

        // DCIALL to invalidate L2 cache bank (needs to be run 4 times, once per bank)
        // This must be done early in code (prior to enabling the caches)
        "MOV r0, #0x2\n"
        "MCR p15, 0, r0, c9, c0, 6\n"       // DCIALL bank D ([15:14] == 2'b00)
        "ORR r0, r0, #0x00004000\n"
        "MCR p15, 0, r0, c9, c0, 6\n"       // DCIALL bank C ([15:14] == 2'b01)
        "ADD r0, r0, #0x00004000\n"
        "MCR p15, 0, r0, c9, c0, 6\n"       // DCIALL bank B ([15:14] == 2'b10)
        "ADD r0, r0, #0x00004000\n"
        "MCR p15, 0, r0, c9, c0, 6\n"       // DCIALL bank A ([15:14] == 2'b11)
        // DCIALL to invalidate entire D-Cache
        "MOV r0, #0\n"
        "MCR p15, 0, r0, c9, c0, 6\n"       // DCIALL r0
        "DSB\n"
        "ISB\n"

        // Invalidate the UTLB
        "MOV r0, #0\n"
        "MCR p15, 0, r0, c8, c7, 0\n"       // UTLBIALL
        "ISB\n"

        ".ltorg\n"
    );
}

void htcleo_disable_interrupts(void)
{
	//clear current pending interrupts
    MmioWrite32(VIC_INT_CLEAR0, 0xffffffff);
    MmioWrite32(VIC_INT_CLEAR1, 0xffffffff);

	//disable all
    MmioWrite32(VIC_INT_EN0, 0);
    MmioWrite32(VIC_INT_EN1, 0);
	//disable interrupts
    MmioWrite32(VIC_INT_MASTEREN, 0);
}

void htcleo_boot(void* kernel,unsigned machtype,void* tags)
{
	htcleo_disable_interrupts();
	htcleo_boot_s(kernel, machtype, tags);
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

void boot_linux(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,void *kernel, unsigned *tags, 
		const char *cmdline, unsigned machtype,
		void *ramdisk, unsigned ramdisk_size)
{
	EFI_STATUS Status;
    UINTN MemoryMapSize = 0;
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
	unsigned *ptr = tags;
	//unsigned pcount = 0;
	//void (*entry)(unsigned,unsigned,unsigned*) = kernel;
	//struct ptable *ptable;
	int cmdline_len = 0;
	int have_cmdline = 0;
	//int pause_at_bootup = 0;

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
	if (cmdline)
	   Print(L"cmdline: %s\n", cmdline);

    /*ToDo: add uefi version of this
	 arch_disable_cache(UCACHE);
	 arch_disable_mmu();
    */

    /*replace with crappy fade effect code i guess*/
// #if DISPLAY_SPLASH_SCREEN
// 	display_shutdown();
// #endif


    // Get the size of the memory map
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        Print(L"Failed to get memory map size: %r\n", Status);
    }

    // Allocate enough memory for the memory map
    Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, MemoryMapSize, (void **)&MemoryMap);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory for memory map: %r\n", Status);
    }

    // Get the actual memory map
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get memory map: %r\n", Status);
    }

    // Exit Boot Services
    Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to exit boot services: %r\n", Status);
    }

	//we are ready to boot the freshly loaded kernel
	htcleo_boot(kernel, machtype, tags);
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
 //load kernel and ramdisk and atags in memory here and call boot_linux with it
EFI_STATUS Status;
VOID *KernelBuffer;
UINTN KernelSize;
VOID *RamdiskBuffer;
UINTN RamdiskSize;
CHAR16 *KernelPath = L"\\zImage";
CHAR16 *RamdiskPath = L"\\initrd.img";
//ToDo: get load address dynamically
VOID *KernelLoadAddress = (VOID *)0x11808000;
VOID *RamdiskLoadAddress = (VOID *)0x12200000;
unsigned *tags_address= (unsigned *)0x11800100;
const char *cmdline = "androidboot.hardware=htcleo androidboot.selinux=permissive androidboot.configfs=false";
unsigned machtype = 0x9dc;

Status = SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
ASSERT_EFI_ERROR(Status);
Status = LoadFileFromSDCard(ImageHandle, SystemTable, KernelPath, KernelLoadAddress, &KernelBuffer, &KernelSize);
if (EFI_ERROR(Status)) {
    Print(L"Failed to load kernel: %r\n", Status);
} else {
     Print(L"Kernel loaded successfully at address %p. Size: %d bytes\n", KernelLoadAddress, KernelSize);
	 //load ramdisk into memory at ramdisk addr
	 Status = LoadFileFromSDCard(ImageHandle, SystemTable, RamdiskPath, RamdiskLoadAddress, &RamdiskBuffer, &RamdiskSize);
	if (EFI_ERROR(Status)) {
    Print(L"Failed to load kernel: %r\n", Status);
	} else {
		Print(L"Ramdisk loaded successfully at address %p. Size: %d bytes\n", RamdiskLoadAddress, RamdiskSize);
	}
    // Now you can use KernelBuffer as needed
    
    // Example: Print the first few bytes of the kernel buffer
    //  UINTN PrintSize = (KernelSize > 16) ? 16 : KernelSize;  // Print at most 16 bytes
    //  for (UINTN i = 0; i < PrintSize; i++) {
    //      Print(L"%02X ", ((UINT8*)KernelBuffer)[i]);
    //  }
    //  Print(L"\n");

	//  PrintSize = (RamdiskSize > 16) ? 16 : RamdiskSize;  // Print at most 16 bytes
    //  for (UINTN i = 0; i < PrintSize; i++) {
    //      Print(L"%02X ", ((UINT8*)RamdiskBuffer)[i]);
    //  }
    //  Print(L"\n");
	//  for(;;);

    
    
	//kernel and ramdisk and cmdline can now be read from sdcard
	boot_linux(ImageHandle, SystemTable, KernelBuffer, tags_address,cmdline,machtype, RamdiskBuffer, RamdiskSize);
	// Clean up the kernel buffer when no longer needed, but we should never reach here essentially halt the platform
	    FreePool(KernelBuffer);
		Print(L"Booting Linux Failed, unkown error occured");
		for(;;);
}
	
}

