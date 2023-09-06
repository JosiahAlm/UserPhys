#pragma once
#include <Windows.h>
#include <iostream>
struct NtCall {
    NtCall(const char* FunctionName, __int64 DataPtr) {
        this->FunctionName = FunctionName;
        this->DataPtr = DataPtr;
        CallAddress = GetProcAddress(LoadLibraryA("win32u.dll"), this->FunctionName);
    }
    NtCall() { }
    const char* FunctionName;
    __int64 DataPtr;
    void* CallAddress;
};
typedef struct ASUSCall
{
    int InterFaceType;
    int InterFaceSize;
    __int64 PhysicalAddress;
    int AddressSpace;
    int SIZE;
    __int64 BaseAddress;
};
typedef struct _PHYSICAL_ADDRESS {
    union {
        struct {
            ULONG LowPart;
            LONG HighPart;
        } DUMMYSTRUCTNAME;
        struct {
            ULONG LowPart;
            LONG HighPart;
        } u;
        ULONGLONG QuadPart;
    } DUMMYUNIONNAME;
} PHYSICAL_ADDRESS, * PPHYSICAL_ADDRESS;

typedef struct _MM_COPY_ADDRESS {
    union {
        PVOID            VirtualAddress;
        PHYSICAL_ADDRESS PhysicalAddress;
    };
} MM_COPY_ADDRESS, * PMMCOPY_ADDRESS;