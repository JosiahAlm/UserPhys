# Kernel Exploitation Library: Advanced UM->KM Function Execution
This framework provides a modern approach to kernel exploitation, leveraging the ASUS Driver AsUpIo64 for initial physical read/write operations enabling the program to grant it self the ability to directly execute kernel functions from user-mode through the abuse of windows syscalls.

# Features
1. **Exploitation of ASUS Driver AsUpIo64:** Utilizes the ASUS Driver AsUpIo64 for initial physical read/write operations thus staging the exploitation of Windows syscalls for direct kernel function execution.
2. **UM kernel function execution:** This technique evades traditional detection mechanisms used by anti-viruses software, by offering a modern approach that moves away from the conventional ".data Pointer" UM->KM communication removing by far the most trivial to detect aspect.
3. **Direct Kernel Function Calls from User Mode:** This technique allows for direct calls to kernel functions from user mode without the need for a handler in kernel memory, diverging from the conventional ".data pointer" drivers methodology and making the task of detecting this execution marginally harder
4. **Wrappers for commonly used functions:** This project includes recreated versions for commonly used functions such as PsLookupProcessByProcessId or PsLookupThreadByThreadId by recreating there exact methods of retreiving there return data, utilizing physical memory read/write. Thus making monitoring of this programs execuation once gain even more challenging

# Traditional UM->KM Communication and Function Execution and where its fundamentally flawed
Historically, malware has sought kernel-level access through various methods, including the use of IOCTLs, hooking syscall functions, and abuse of pointer execution within syscall functions. Regardless of the specific technique, the initial steps of these approaches often follow a consistent procedure:

1. **Utilization of a Vulnerable Driver:** The initial step involves identifying and exploiting a vulnerable driver with the capability to read/write physical memory.
3. **Memory Allocation:** Once a driver has been identified and successfully exploited kernel memory is allocated. While there are multiple ways this can be done the typical method seems to be thorugh allocation of a NonPagedPool via ExAllocatePoolWithTag ensuring that the allocated memory doesn't get paged out, making it consistently accessible.
4. **Resolving Relocations and Imports:** Prior to transferring the driver to the allocated kernel memory, its relocations and imports are resolved this is a typical step in manually mapping anything.
5. **Execution of the Entry Point:** With the preparations complete, the driver is transferred from its user-mode process buffer to the previously allocated memory. The entry point of the driver is then typically executed by setting a syscall function to jump to the entry point. This syscall function is subsequently called from user mode to initialize the driver.

**Understanding Initial Function Execution**


To gain the ability to execute functions like **ExAllocatePoolWithTag**, attackers typically follow a straightforward process:

1. **Identify a Rarely Used Nt/Syscall Function:** The first step is to pinpoint a syscall function that the system rarely calls. A well-known example of such a function is NtAddAtom.
3. **Overwrite the Function with Shellcode:** With the function identified, the attacker leverages a vulnerable driver's capability to write to physical memory. They then overwrite the function's existing code with shellcode. A common set of instructions used for this purpose is **mov RAX, AddressOfFunction; jmp RAX.**
4. **Execute the Overwritten Function:** By doing this, the attacker can now execute NtAddAtom, but it behaves as if it were another function, like ExAllocatePoolWithTag or the Entry Point of a driver as stated below.

**The Flaw**


From what ive seen many individuals in the field seem to have a tunnel vision focus on UM->KM communication, innovating new methods for this purpose. While these techniques can make tracking of execution challenging, there's an overlooked vulnerability inherent to manually mapping a driver. Specifically, the driver and any hook handlers (if one chooses that method of UM->KM communication) now reside in unsigned kernel memory. To the best of my knowledge, this is an anomaly that shouldn't occur under normal circumstances and can be flagged by a kernel level anti-virus very rather easily. What i did notice while not quite as hard to track a simler technique to what this project displays is often employed to call kernel functions. However, it's not fully capitalized upon in the manner it potentially could be. As highlighted in **Understanding Initial Function Execution**, the use of syscalls for kernel function execution is prevalent yet the chosen method for many, specifically **mov RAX, AddressOfFunction; jmp RAX** or any other patching within the .text section of any driver can be easily flagged by kernel-level anti-viruses by simply comparing the .text section loaded in kernel memory to its counterpart on disk. Given that this section only has Read and Execute page protection, it remains immutable, making any deviations a clear red flag.

# Abstract:
This project leverages an often-overlooked aspect of the Windows operating system. Specifically, it focuses on the behavior of **win32k.sys**, a Windows driver loaded into **ntoskrnl.exe** by **winload.efi**. This driver is responsible for receiving syscall execution for before delegating them to their respective drivers. The process can be visualized as:


![[image]](https://github.com/JosiahAlm/UserPhys/blob/main/img/Redirect.png)


With the call stack represented as:


![[image]](https://github.com/JosiahAlm/UserPhys/blob/main/img/CallStack.png)


By exploiting this mechanism, one can write to the specified qword, redirecting it to the address of a desired kernel function. This grants the ability to execute any kernel function without the need to modify the .text section of a driver or rely on a handler mapped into the kernel both being trival to detect as stated before, thus streamlining the process and reducing potential points of detection.

**Why This is So Challenging to Mitigate**

The driving factor behind this "Vulnerability" and what makes it so hard for anti-viruses to mitigate or track once loaded relys on many short comings of attempting to parse win32k.sys without manual intervention one of the main issues being locating these NT function handlers since the exports of **win32k.sys** do not directly point to them:

![[image]](https://github.com/JosiahAlm/UserPhys/blob/main/img/StubExport.png)

![[image]](https://github.com/JosiahAlm/UserPhys/blob/main/img/stubPsudo.png)


If **win32k.sys** were structured such that its exports directly referenced each corresponding function, mitigation would be more straightforward. Anti-virus solutions could:
1. **Iterate Through Exports:** Loop through the exports of win32k.sys.
2. **Driver Location Verification:** Determine which driver the function should point to and verify if the qword within is indeed referencing the correct location.


While this approach might be computationally intensive, potentially affecting system performance, it offers a direct way to counteract the exploit.


However, the current setup doesn't afford this luxury as far as i know. Without direct references, a more labor-intensive approach becomes necessary:
1. **Manual Definition:** One would need to manually define which qword corresponds to each function.
2. **Follow the above steps:** After this legnth task has been done you could then follow the above steps but once again you run into the problem of computationally intensity.

**Version-Specific Mitigation**

Given the frequent updates and variations of Windows versions where **win32k.sys** might be affected, this task becomes even more daunting. Each version would require its unique set of definitions.
