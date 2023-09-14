#include "Syscall.h"
NTSTATUS Syscall::MmCopyMemory(PVOID TargetAddress, MM_COPY_ADDRESS SourceAddress, SIZE_T NumberOfBYtes, ULONG Flags, PSIZE_T NumOfBytesTrans) {
    return Call<NTSTATUS>(_MmCopyMemory.CallAddress, TargetAddress, SourceAddress, NumberOfBYtes, Flags, NumOfBytesTrans);
}
PVOID Syscall::MmMapIoSpaceEx(PHYSICAL_ADDRESS TargetAddress, SIZE_T NumberOfBYtes, ULONG Protect) {
	return Call<PVOID>(_MmMapIoSpaceEx.CallAddress, TargetAddress, NumberOfBYtes, Protect);
}
void Syscall::MmUnmapIoSpace(PVOID TargetAddress, SIZE_T NumberOfBYtes) {
	Call<void>(_MmUnmapIoSpace.CallAddress, TargetAddress, NumberOfBYtes);
}
void* Syscall::K_memcpy(void* dst, void* src, int size) {
    return Call<void*>(_memcpy.CallAddress, dst, src, size);

}

bool Syscall::ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size) {
    if (!PhysicalAddr) return false;
    if (!Buffer) return false;
    if (!size) return false;

    MM_COPY_ADDRESS Phys = { 0 }; Phys.PhysicalAddress.QuadPart = (UINT64)PhysicalAddr;
    SIZE_T Read = 0;
    NTSTATUS Status = MmCopyMemory(Buffer, Phys, size, 0x1, &Read);
    return NT_SUCCESS(Status);
}
bool Syscall::WritePhysical(__int64 PhysicalAddr, void* Buffer, int size) {
    if (!PhysicalAddr) return false;
    if (!Buffer) return false;
    if (!size) return false;
    SIZE_T Wrote;

    PHYSICAL_ADDRESS Phys = { 0 }; Phys.QuadPart = PhysicalAddr;
    void* MappedAddress = MmMapIoSpaceEx(Phys, size, PAGE_READWRITE);
    if (!MappedAddress) return false;

    MM_COPY_ADDRESS Virtual = { 0 }; Virtual.VirtualAddress = Buffer;
    NTSTATUS Status = MmCopyMemory(MappedAddress, Virtual, size, 0x2, &Wrote);
    MmUnmapIoSpace(MappedAddress, size);

    return NT_SUCCESS(Status) && Wrote;
}