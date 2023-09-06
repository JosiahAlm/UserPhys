# Kernel Exploitation Library: Advanced UM->KM Function Execution
This framework provides a modern approach to kernel exploitation, leveraging the ASUS Driver AsUpIo64 for initial physical read/write operations enabling the program to grant it self the ability to directly execute kernel functions from user-mode.

# Features
1. Exploitation of ASUS Driver AsUpIo64: Utilizes the ASUS Driver AsUpIo64 for initial physical read/write operations, laying the groundwork for advanced kernel exploitation tasks.
2. Stealthy UM->KM Communication: Evades traditional detection mechanisms used by anti-viruses, offering a cutting-edge method that moves away from the conventional ".data Pointer" approach.
3. Direct Kernel Function Calls from User Mode: Allows for direct calls to kernel functions from user mode without the need for a handler in kernel memory, a departure from traditional ".data pointer" drivers.

