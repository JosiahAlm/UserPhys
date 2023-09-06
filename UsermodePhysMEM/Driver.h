#pragma once
#include <windows.h>
#include <iostream>
#include "structs.h"
class Driver
{
public:
    Driver() { }
    Driver(const char* DriverLocation, const char* ServiceName) {
        this->DriverLocation = DriverLocation;
        this->ServiceName = ServiceName;
    }
    NTSTATUS CreateDriver();
    NTSTATUS DeleteDriver();

    NTSTATUS StartDriver();
    NTSTATUS StopDriver();

    bool ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size);
    bool WritePhysical(__int64 PhysicalAddr, void* Buffer, int size);
private:
    const char* DriverLocation;
    const char* ServiceName;
    HANDLE DriverHandle;
};

