#pragma once
#include "structs.h"

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
class Syscall {
public:
	Syscall() { }
	Syscall(NtCall MmCopyMemory, NtCall MmMapIoSpaceEx, NtCall MmUnmapIoSpace) {
		this->_MmCopyMemory = MmCopyMemory;
		this->_MmMapIoSpaceEx = MmMapIoSpaceEx;
		this->_MmUnmapIoSpace = MmUnmapIoSpace;
		LoadLibraryA("user32.dll");
		LoadLibraryA("win32u.dll");
		LoadLibraryA("ntdll.dll");

	}
	NTSTATUS MmCopyMemory(PVOID TargetAddress, MM_COPY_ADDRESS SourceAddress, SIZE_T NumberOfBYtes, ULONG Flags, PSIZE_T NumOfBytesTrans);
	PVOID MmMapIoSpaceEx(PHYSICAL_ADDRESS TargetAddress, SIZE_T NumberOfBYtes, ULONG Protect);
	void MmUnmapIoSpace(PVOID TargetAddress, SIZE_T NumberOfBYtes);

	bool ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size);
	bool WritePhysical(__int64 PhysicalAddr, void* Buffer, int size);
private:
	NtCall _MmCopyMemory;
	NtCall _MmMapIoSpaceEx;
	NtCall _MmUnmapIoSpace;
	template <typename ReturnType, typename... Args>
	ReturnType Call(void* Addr, Args... Arguments) { return ((ReturnType(*)(Args...))(Addr))(Arguments...); }
};

