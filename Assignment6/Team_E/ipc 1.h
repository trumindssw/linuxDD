#ifndef IPC_H              // Prevent multiple inclusions of this header file
#define IPC_H

// This file will be used to create shared memory between sender and receiver.
#define FILENAME "/tmp/ipc_mmap_file"

// Define the size of the memory-mapped region (in bytes).
#define MMAP_SIZE 1024

// Define a signal number for inter-process signaling.
// SIGUSR1 is a user-defined signal, typically used for custom IPC needs.
#define SIGNAL_NUM SIGUSR1

#endif

