#pragma once
#include <windows.h>
#include "structs.h"
#include "status.h"


class Driver
{
public:
    Driver() = default;
    Driver(const char* DriverLocation, const char* ServiceName) : _DriverLocation(DriverLocation), _ServiceName(ServiceName) { }
    NTSTATUS CreateDriver();
    NTSTATUS DeleteDriver();

    NTSTATUS StartDriver();
    NTSTATUS StopDriver();

    bool ReadPhysical(__int64 PhysicalAddr, void* Buffer, int Size);
    bool WritePhysical(__int64 PhysicalAddr, void* Buffer, int Size);
private:
    const char* _DriverLocation;
    const char* _ServiceName;
    HANDLE _DriverHandle;

    SC_HANDLE OpenSCM();
    SC_HANDLE OpenDriverService(SC_HANDLE hSCM);
    void CloseHandles(SC_HANDLE hService, SC_HANDLE hSCM);
};