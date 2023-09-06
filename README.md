# Kernel Exploitation Library: Advanced UM->KM Function Execution
This framework provides a modern approach to kernel exploitation, leveraging the ASUS Driver AsUpIo64 for initial physical read/write operations enabling the program to grant it self the ability to directly execute kernel functions from user-mode through the abuse of windows syscalls.

# Features
1. **Exploitation of ASUS Driver AsUpIo64:** Utilizes the ASUS Driver AsUpIo64 for initial physical read/write operations thus staging the exploitation of Windows syscalls for direct kernel function execution.
2. **UM kernel function execution:** This technique evades traditional detection mechanisms used by anti-viruses software, by offering a modern approach that moves away from the conventional ".data Pointer" UM->KM communication removing by far the most trivial to detect aspect.
3. **Direct Kernel Function Calls from User Mode:** This technique allows for direct calls to kernel functions from user mode without the need for a handler in kernel memory, diverging from the conventional ".data pointer" drivers methodology and making the task of detecting this execution marginally harder
4. **Wrappers for commonly used functions:** This project includes recreated versions for commonly used functions such as PsLookupProcessByProcessId or PsLookupThreadByThreadId by recreating there exact methods of retreiving there return data, utilizing physical memory read/write. Thus making monitoring of this programs execuation once gain even more challenging

# Abstract: Traditional UM->KM Communication and Function Execution
Historically, malware has sought kernel-level access through various methods, including the use of IOCTLs, hooking syscall functions, and abuse of pointer execution within syscall functions. Regardless of the specific technique, the initial steps of these approaches often follow a consistent procedure:

1. **Utilization of a Vulnerable Driver:** The initial step involves identifying and exploiting a vulnerable driver with the capability to read/write physical memory.
3. **Memory Allocation:** Once a driver has been identified and successfully exploited kernel memory is allocated. While there are multiple ways this can be done the typical method seems to be thorugh allocation of a NonPagedPool via ExAllocatePoolWithTag ensuring that the allocated memory doesn't get paged out, making it consistently accessible.
4. **Resolving Relocations and Imports:** Prior to transferring the driver to the allocated kernel memory, its relocations and imports are resolved this is a typical step in manually mapping anything.
5. **Execution of the Entry Point:** With the preparations complete, the driver is transferred from its user-mode process buffer to the previously allocated memory. The entry point of the driver is then typically executed by setting a syscall function to jump to the entry point. This syscall function is subsequently called from user mode to initialize the driver.
