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

#include <stdio.h>      // For printf, fprintf, perror
#include <stdlib.h>     // For atoi, exit
#include <fcntl.h>      // For open() and file access flags
#include <unistd.h>     // For close(), ftruncate()
#include <sys/mman.h>   // For mmap()
#include <string.h>     // For string functions
#include <time.h>       // For getting current date/time
#include <signal.h>     // For sending signals with kill()
#include "ipc.h"        // Shared definitions (FILENAME, MMAP_SIZE, SIGNAL_NUM)

int main(int argc, char *argv[]) {
    // Check if user provided the receiver process ID
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <receiver-pid>\n", argv[0]);
        return 1;
    }

    // Convert receiver PID from string to integer
    pid_t receiver_pid = atoi(argv[1]);

    // Open or create the file that will be used for mmap (shared memory)
    int fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("open");  // Print error if file cannot be opened
        return 1;
    }

    // Set the file size so mmap can map the full MMAP_SIZE
    ftruncate(fd, MMAP_SIZE);

    // Create a memory-mapped area from the file
    char *addr = mmap(NULL, MMAP_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");  // Print error if mmap fails
        return 1;
    }

    // Get current time and format it as a string
    time_t now = time(NULL);
    char *timestr = ctime(&now);  // Converts time to a readable string

    // Write the sender's PID and the current time into shared memory
    snprintf(addr, MMAP_SIZE, "Sender PID: %d\nDate/Time: %s", getpid(), timestr);

    // Send a signal to the receiver process to let it know data is ready
    kill(receiver_pid, SIGNAL_NUM);

    // Clean up: unmap memory and close file
    munmap(addr, MMAP_SIZE);
    close(fd);

    return 0;  // Exit successfully
}

