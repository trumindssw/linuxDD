/*Team - B
Devi, Keerthana 
Assignment - 5
*/

/*
Question:
---------
Develop and understanding of the /proc system and how the 
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#define MAP_SIZE (64 * 1024 * 1024)  /* Define size for mmap: 64 MB */

/*
 * Function: mmap_is_enabled
 * -------------------------
 * Reads the control flag from /tmp/mmap_control.
 *
 * Return:
 *     1 if mmap should run (file contains '1' or unreadable),
 *     0 if mmap should not run (file contains '0')
 */

static int mmap_is_enabled(void)
{
	FILE *file;
	char flag;
	size_t bytes_read;

	// Attempt to open the control file for reading
	file = fopen("/tmp/mmap_control", "r");
	if (!file) 
	{
		// If file can't be opened, default to enabling mmap
		perror("Warning: Could not open /tmp/mmap_control (defaulting to enabled)");
		return 1;
	}

	// Read the first byte from the file
	bytes_read = fread(&flag, 1, 1, file);
	fclose(file);

	// If reading fails, log an error and return default (enabled)
	if (bytes_read != 1) 
	{
		fprintf(stderr, "Error: Could not read flag from /tmp/mmap_control\n");
		return 1;
	}

	// Return 1 if the flag is '1', otherwise 0
	return (flag == '1');
}

int main(void)
{
	void *mapped_addr;

	// Check control file to see if mmap should be performed
	if (!mmap_is_enabled()) 
	{
		printf("mmap is DISABLED via control file (/tmp/mmap_control).\n");
		return EXIT_SUCCESS;
	}

	// Request 64MB of anonymous memory, readable and writable
	mapped_addr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mapped_addr == MAP_FAILED) 
	{
		// If mmap fails, print error and exit
		perror("Error: mmap failed");
		return EXIT_FAILURE;
	}

	// Log the address of the mapped memory
	printf("Mapped 64MB at address: %p\n", mapped_addr);

	//Fill the mapped memory region with pattern 0xAA
	memset(mapped_addr, 0xAA, MAP_SIZE);
	printf("Filled mapped memory with pattern 0xAA\n");

	// Pause the program to allow inspection of /proc/[pid]/maps or /proc/[pid]/smaps
	printf("Press Enter to unmap and exit...\n");
	getchar();

	// Clean up: unmap the memory region
	if (munmap(mapped_addr, MAP_SIZE) != 0) 
	{
		perror("Error: munmap failed");
		return EXIT_FAILURE;
	}

	printf("Memory unmapped successfully.\n");
	return EXIT_SUCCESS;
}
