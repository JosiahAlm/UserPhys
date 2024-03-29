#pragma once
#include "structs.h"

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
class Syscall {
public:
	Syscall() = default;
	Syscall(NtCall MmCopyMemory, NtCall MmMapIoSpaceEx, NtCall MmUnmapIoSpace, NtCall memcpy) :
		_MmCopyMemory(MmCopyMemory),
		_MmMapIoSpaceEx(MmCopyMemory),
		_MmUnmapIoSpace(MmCopyMemory),
		_memcpy(memcpy)
	{
		LoadLibraryA("user32.dll");
		LoadLibraryA("win32u.dll");
		LoadLibraryA("ntdll.dll");
	}
	NTSTATUS MmCopyMemory(PVOID TargetAddress, MM_COPY_ADDRESS SourceAddress, SIZE_T NumberOfBYtes, ULONG Flags, PSIZE_T NumOfBytesTrans);
	PVOID MmMapIoSpaceEx(PHYSICAL_ADDRESS TargetAddress, SIZE_T NumberOfBYtes, ULONG Protect);
	void MmUnmapIoSpace(PVOID TargetAddress, SIZE_T NumberOfBYtes);
	void* K_memcpy(void* dst,void* src,int size);

	bool ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size);
	bool WritePhysical(__int64 PhysicalAddr, void* Buffer, int size);
private:
	NtCall _MmCopyMemory;
	NtCall _MmMapIoSpaceEx;
	NtCall _MmUnmapIoSpace;
	NtCall _memcpy;
	template <typename ReturnType, typename... Args>
	ReturnType Call(void* Addr, Args... Arguments) { return ((ReturnType(*)(Args...))(Addr))(Arguments...); }
};

