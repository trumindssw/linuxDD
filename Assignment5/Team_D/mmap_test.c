/* TEAM_D
 
 *Updated  ASSIGNMENT-5
 
 * Shravani , Naveen.
 
 * ASSIGNMENT - 5 QUESTION
 
 * Develop and understanding of the /proc system and how the
* Memory
* File System
* IO are manipulated using the same
How to make it persistent across reboots ? - once these are identified, then adding them into the Linux init scripts
1. Create a code that uses mmap and maps a 64MB memory
2. Update the parameters (thru command line) using /proc and DISABLE the mmap
3. Run the code created in Step (1) above and see how it works 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
 
#define MAP_SIZE (64 * 1024 * 1024) // 64MB
#define FILE_PATH "mapped_file.bin"
#define TRIGGER_VALUE 999999UL
 
unsigned long read_proc_shmmax() {
    FILE *fp = fopen("/proc/sys/kernel/shmmax", "r");
    if (!fp) {
        perror("fopen /proc/sys/kernel/shmmax");
        return 0;
    }
 
    unsigned long value;
    if (fscanf(fp, "%lu", &value) != 1) {
        perror("fscanf");
        fclose(fp);
        return 0;
    }
 
    fclose(fp);
    return value;
}
 
int main() {
    unsigned long shmmax = read_proc_shmmax();
    if (shmmax == TRIGGER_VALUE) {
        printf("mmap is disabled.\n");
        return 0;
    }
 
    printf("mmap is enabled.\n");
 
    // 1. Create and open the file
    int fd = open(FILE_PATH, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }
 
    // 2. Make file large enough
    if (ftruncate(fd, MAP_SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }
 
    // 3. Memory map the file
    void *mapped = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
 
    printf("64MB file-backed memory mapped at %p\n", mapped);
 
    memset(mapped, 0xA5, MAP_SIZE);
    printf("Memory initialized with 0xA5 pattern\n");
 
    printf("Monitoring /proc/sys/kernel/shmmax for trigger value %lu...\n", TRIGGER_VALUE);
    while (1) {
        shmmax = read_proc_shmmax();
        if (shmmax == TRIGGER_VALUE) {
            printf("Trigger detected! Unmapping memory.\n");
            break;
        }
        sleep(1);
    }
 
    // 4. Cleanup
    munmap(mapped, MAP_SIZE);
    close(fd);
    printf("Memory unmapped and file closed successfully.\n");
 
    return 0;
}
