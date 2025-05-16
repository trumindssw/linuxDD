/*Team - E
A.Prasanthi, Ramya, Prashanth
A6
*/

/*
Question:
---------
IPC Code (with MAKEIFLE, C, .h files ONLY)
1. Create a shared memory segment between two processes.
2. Use signals to synchronize access
3. Create a mmap and let it be accessible by 2 Processes - separate processes - call it sender and receiver
4. Sender writes current date / time on the mmaped memory or file, sets the Signal (self defined signal number create any)
5. Receiver is in a tight loop listening to the signals (need a signal handler here)
6. once the receiver handles the signal, it reads the data from the file and prints it on the command line
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include "ipc.h"

char *addr;     // Pointer to shared memory
int fd;         // File descriptor
volatile sig_atomic_t done = 0;  // Flag set by signal handler

// Signal handler for SIGUSR1
void handle_signal(int sig) {
    if (sig == SIGNAL_NUM) {
        printf("Received signal %d (SIGUSR1)\n", sig);
        printf("Data from mmap:\n%s\n", addr);
        done = 1;  // Set flag to exit loop
    }
}

int main() {
    // Register signal handler
    signal(SIGNAL_NUM, handle_signal);

    // Open or create the shared memory file
    fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Set size of the file
    ftruncate(fd, MMAP_SIZE);

    // Memory map the file
    addr = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Receiver PID: %d — Waiting for signal...\n", getpid());

    // Wait for signal
    while (!done) {
        pause();  // Sleep until a signal is received
    }

    // Cleanup
    munmap(addr, MMAP_SIZE);
    close(fd);

    printf("Receiver exiting after handling signal.\n");
    return 0;
}

