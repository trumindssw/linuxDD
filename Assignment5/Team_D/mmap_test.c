/* TEAM_D
 
 *  ASSIGNMENT-5
 
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
 
#define SIZE (64 * 1024 * 1024) // 64MB
 
 
int main() {
        void *map;
        //  mmap 64MB
        map = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (map == MAP_FAILED) {
                perror("mmap failed, can't access the memory");
                return EXIT_FAILURE;
        }
 
        printf("64MB mapped at address %p\n", map);
 
        // Fill with pattern
        memset(map, 0xA5, SIZE);
        printf("Memory initialized with 0xA5\n");
 
        //  Wait for user input before unmapping
        printf("Press Enter to unmap the memory...\n");
        getchar();
 
        // Unmap
        if (munmap(map, SIZE) == -1) {
                perror("munmap failed");
        } else {
                printf("Memory successfully unmapped.\n");
        }
        return 0;
}
