#include "Kernel.h"

int Kernel::ProcessId(const char* Name, int Index) {
    int I = 0;
    __int64 ExportedAddress = KernelExport("PsInitialSystemProcess");
    __int64 InitialSystemProcess = Read<__int64>(TranslateLinearAddress(DirTableBase, (__int64)ExportedAddress));
    __int64 Entry = InitialSystemProcess;
    do
    {
        char image_name[15];
        ReadPhysical(TranslateLinearAddress(DirTableBase, (__int64)(Entry + 0x5a8)), &image_name, sizeof(image_name)); // EPROCESS->ImageFileName
        if (!strcmp(Name, image_name)) {
            int ActiveThreads = Read<int>(TranslateLinearAddress(DirTableBase, (__int64)(Entry + 0x5f0)));
            if (ActiveThreads) {
                I++;
                if (I == Index) {
                    return Read<int>(TranslateLinearAddress(DirTableBase, (__int64)(Entry + 0x440)));;
                }
            }
        }
        __int64 Flink = Read<__int64>(TranslateLinearAddress(DirTableBase, Entry + 0x448));
        Entry = Flink - 0x448;
    } while (Entry != InitialSystemProcess);
    return 0;
}
__int64 Kernel::BruteKernelDirectoryTable() {
    __int64 Result = 0x00;

    for (__int64 Index = 0; Index < 100; Index++)
    {
        BYTE Buffer[0x1000];

        if (!ReadPhysical(0x1000 + (Index * 0x1000), Buffer, sizeof(Buffer)))
            continue;

        if (0x00000001000600E9 ^ (0xffffffffffff00ff & *(__int64*)(Buffer)))
            continue;

        if (0xfffff80000000000 ^ (0xfffff80000000000 & *(__int64*)(Buffer + 0x70)))
            continue;

        if (0xffffff0000000fff & *(__int64*)(Buffer + 0xA0))
            continue;

        Result = *(__int64*)(Buffer + 0xA0);

        break;
    }
    return Result;
}
__int64 Kernel::EProcess(const char* Name, int Index) {
    int I = 0;
    __int64 ExportedAddress = KernelExport("PsInitialSystemProcess");
    __int64 InitialSystemProcess = Read<__int64>(TranslateLinearAddress(DirTableBase, (__int64)ExportedAddress));
    __int64 Entry = InitialSystemProcess;
    do
    {
        char image_name[15];
        ReadPhysical(TranslateLinearAddress(DirTableBase, (__int64)(Entry + 0x5a8)), &image_name, sizeof(image_name)); // EPROCESS->ImageFileName
        if (!strcmp(Name, image_name)) {
            int ActiveThreads = Read<int>(TranslateLinearAddress(DirTableBase, (__int64)(Entry + 0x5f0)));
            if (ActiveThreads) {
                I++;
                if (I == Index) {
                    return Entry;
                }
            }
        }
        __int64 Flink = Read<__int64>(TranslateLinearAddress(DirTableBase, Entry + 0x448));
        Entry = Flink - 0x448;
    } while (Entry != InitialSystemProcess);
    return 0;
}
__int64 Kernel::EProcess(int TragetProcessId) {
    __int64 ExportedAddress = KernelExport("PsInitialSystemProcess");
    __int64 InitialSystemProcess = Read<__int64>(TranslateLinearAddress(DirTableBase, (__int64)ExportedAddress));
    __int64 Entry = InitialSystemProcess;
    do
    {
        int ProcessId = Read<int>(TranslateLinearAddress(DirTableBase, (__int64)(Entry + 0x440)));
        if (ProcessId == TragetProcessId) {
            int ActiveThreads = Read<int>(TranslateLinearAddress(DirTableBase, (__int64)(Entry + 0x5f0)));
            if (ActiveThreads) {
                return Entry;
            }
        }
        __int64 Flink = Read<__int64>(TranslateLinearAddress(DirTableBase, Entry + 0x448));
        Entry = Flink - 0x448;
    } while (Entry != InitialSystemProcess);
    return 0;
}
__int64 Kernel::EThread(__int64 DirectoryTable, __int64 EProcess, int TargetThreadId) {
    __int64 ThreadListHead = Read<__int64>(TranslateLinearAddress(DirectoryTable, EProcess + 0x5e0)) - 0x4e8; //(EProcess->ThreadListHead) - Ethread->ThreadListEntry
    __int64 Entry = ThreadListHead;
    do {
        __int64 UniqueThread = Read<__int64>(TranslateLinearAddress(DirectoryTable, Entry + 0x478 + 0x8)); //Ethread->Cid.UniqueThread
        if ((__int64)TargetThreadId == UniqueThread) {
            return Entry;
        }
        __int64 Flink = Read<__int64>(TranslateLinearAddress(DirectoryTable, Entry + 0x4e8)); //Ethread->ThreadListEntry
        Entry = Flink - 0x4e8;
    } while (Entry != ThreadListHead);
}
__int64 Kernel::ExportOffset(__int64 BaseAddress, const char* FunctionName)
{
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)BaseAddress;
    PIMAGE_NT_HEADERS imageNTHeaders = (PIMAGE_NT_HEADERS)(BaseAddress + dosHeader->e_lfanew);
    UINT64 exportDirectoryRVA = imageNTHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;
    PIMAGE_EXPORT_DIRECTORY imageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(BaseAddress + exportDirectoryRVA);
    unsigned long* addressOfNamesRVA = (unsigned long*)(BaseAddress + imageExportDirectory->AddressOfNames);
    unsigned long* addresOfFunctionsRVA = (unsigned long*)(BaseAddress + imageExportDirectory->AddressOfFunctions);
    unsigned short* addressOfNameOrdinalsRVA = (unsigned short*)(BaseAddress + imageExportDirectory->AddressOfNameOrdinals);
    for (int i = 0; i < imageExportDirectory->NumberOfNames; i++)
    {
        int functionNameRVA = addressOfNamesRVA[i];
        UINT64 functionNameVA = BaseAddress + functionNameRVA;
        char* functionName = (char*)functionNameVA;
        UINT64 functionAddressRVA = addresOfFunctionsRVA[addressOfNameOrdinalsRVA[i]];
        UINT64* functionAddress = (UINT64*)(BaseAddress + functionAddressRVA);
        if (!strcmp(functionName, FunctionName))
        {
            return (UINT64)functionAddress - BaseAddress;
        }
    }
    return NULL;
}
__int64 Kernel::KernelExport(const char* FunctionName)
{
    HMODULE Address = LoadLibraryA("ntoskrnl.exe");
    if (!Address) return 0;
    __int64 ExportedAddress = (__int64)(ntoskrnlVirtual + ExportOffset((__int64)Address, FunctionName));
    FreeLibrary(Address);
    return ExportedAddress;
}
__int64 Kernel::GetProcessDirectoryTable(__int64 EProcess) {
    __int64 DirTable = Read<__int64>(TranslateLinearAddress(DirTableBase, EProcess + 0x28));
    return DirTable;
}
void* Kernel::GetKernelAddress(const char* name)
{
    void* Address = 0;
    DWORD Needed = 0;


    if (K32EnumDeviceDrivers(NULL, 0, &Needed)) {
        LPVOID Drivers[1024];
        DWORD NumDrivers = 0;
        if (K32EnumDeviceDrivers(Drivers, Needed, &NumDrivers)) {
            for (DWORD i = 0; i < NumDrivers; i++) {
                TCHAR driverName[MAX_PATH];
                if (K32GetDeviceDriverBaseNameA(Drivers[i], driverName, sizeof(driverName))) {
                    if (!strcmp(driverName, name)) {
                        Address = Drivers[i];
                        break;
                    }
                }
            }
        }
    }

    return Address;
}
void Kernel::SwapPointer(const char* FunctionName, __int64 Offset) {
    __int64 ExportedAddress = KernelExport(FunctionName);
    Write<__int64>(TranslateLinearAddress(ExplorerDirectoryTable, win32kVirtual + Offset), ExportedAddress);
}

//https://github.com/waryas/UMPMLib/blob/9da1806e3ae3ab9778ce4df886a04ff33ade6c17/MemoryOperationSample/PMemHelper.h#L258
//by far the best function that could be used for this all credits to them
__int64 Kernel::TranslateLinearAddress(__int64 directoryTableBase, __int64 virtualAddress) {
    int16_t pml4 = (virtualAddress >> 39) & 0x1FF;
    int16_t directoryPointer = (virtualAddress >> 30) & 0x1FF;
    int16_t directory = (virtualAddress >> 21) & 0x1FF;
    int16_t table = (virtualAddress >> 12) & 0x1FF;

    uintptr_t pml4e = 0;
    if (!ReadPhysical(directoryTableBase + pml4 * sizeof(uintptr_t), &pml4e, sizeof(pml4e)))
        return 0;
    if (!pml4e)
        return 0;

    uintptr_t pdpte = 0;
    if (!ReadPhysical((pml4e & 0xFFFFFFFFFF000) + directoryPointer * sizeof(uintptr_t), &pdpte, sizeof(pdpte)))
        return 0;

    if (!pdpte)
        return 0;

    if ((pdpte & (1 << 7)) != 0)
        return (pdpte & 0xFFFFFC0000000) + (virtualAddress & 0x3FFFFFFF);

    uintptr_t pde = 0;
    if (!ReadPhysical((pdpte & 0xFFFFFFFFFF000) + directory * sizeof(uintptr_t), &pde, sizeof(pde)))
        return 0;

    if (!pde)
        return 0;

    if ((pde & (1 << 7)) != 0)
        return (pde & 0xFFFFFFFE00000) + (virtualAddress & 0x1FFFFF);

    uintptr_t pte = 0;
    if (!ReadPhysical((pde & 0xFFFFFFFFFF000) + table * sizeof(uintptr_t), &pte, sizeof(pte)))
        return 0;

    if (!pte)
        return 0;

    return (pte & 0xFFFFFFFFFF000) + (virtualAddress & 0xFFF);
}