#include "menu.h"

EFI_STATUS
ReadMemoryAndWriteToFile(
    UINTN* MemoryAddress,
    UINTN Length,
    CHAR16 *FilePath
)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *Root = NULL;
    EFI_FILE_PROTOCOL *FileHandle = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem = NULL;

    // Locate Simple File System Protocol
    Status = gBS->LocateProtocol(
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        (VOID **)&SimpleFileSystem
    );
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Failed to locate Simple File System Protocol: %r\n", Status));
        return Status;
    }

    // Open file system volume
    Status = SimpleFileSystem->OpenVolume(
        SimpleFileSystem,
        &Root
    );
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Failed to open volume: %r\n", Status));
        return Status;
    }

    // Open file using absolute path
    Status = Root->Open(
        Root,
        &FileHandle,
        FilePath,
        EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
        0
    );
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Failed to open file: %r\n", Status));
        Root->Close(Root);
        return Status;
    }

    // Write to file
    Status = FileHandle->Write(
        FileHandle,
        &Length,
        MemoryAddress
    );
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Failed to write to file: %r\n", Status));
    }

    // Close file handles
    FileHandle->Close(FileHandle);
    Root->Close(Root);

    return Status;
}

void DumpDmesg(void)
{
    UINTN Length = 0x40000; // Length provided
    CHAR16 *FilePath = L"\\dmesg.txt"; // Adjust the file path accordingly

    DEBUG((EFI_D_ERROR, "Starting DumpDmesg\n"));
    EFI_STATUS Status = ReadMemoryAndWriteToFile((UINTN*)0x2FFC0000,Length, FilePath);
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Failed to write memory to file: %r\n", Status));
    } else {
        DEBUG((EFI_D_ERROR, "Memory written to %s successfully\n", FilePath));
    }
    DEBUG((EFI_D_ERROR, "DumpDmesg completed\n"));
}

void DumpMemory2Sdcard(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable){
    CHAR16 *hexstring;
    UINTN hexval;
     
    GetHexInput(&hexval, &hexstring,SystemTable);
    UINTN filepath_length = StrLen(L"\\") + StrLen(hexstring) + StrLen(L".bin") + 1;
    CHAR16 *FilePath = AllocatePool(filepath_length * sizeof(CHAR16));
    if (FilePath == NULL) {
        Print(L"Memory allocation failed\n");
    }
    UnicodeSPrint(FilePath, filepath_length * sizeof(CHAR16), L"\\%s.bin", hexstring);
    DEBUG((EFI_D_ERROR, "Starting Memory Dump at 0x%x\n", hexval));
    EFI_STATUS Status = ReadMemoryAndWriteToFile((UINTN*)hexval,0x00100000, FilePath);
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "Failed to write memory to file: %r\n", Status));
    } else {
        DEBUG((EFI_D_ERROR, "Memory written to %s successfully\n", FilePath));
    }
    DEBUG((EFI_D_ERROR, "Memory Dump completed\n"));
}