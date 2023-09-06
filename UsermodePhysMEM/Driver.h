#pragma once
#include <windows.h>
#include <iostream>
#include "structs.h"

class Driver
{
public:
    Driver() = default;
    Driver(const char* DriverLocation, const char* ServiceName) : _DriverLocation(DriverLocation), _ServiceName(ServiceName) { }
    NTSTATUS CreateDriver();
    NTSTATUS DeleteDriver();

    NTSTATUS StartDriver();
    NTSTATUS StopDriver();

    bool ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size);
    bool WritePhysical(__int64 PhysicalAddr, void* Buffer, int size);
private:
    const char* _DriverLocation;
    const char* _ServiceName;
    HANDLE _DriverHandle;
};

