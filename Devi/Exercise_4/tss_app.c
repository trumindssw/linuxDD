// tss_app.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define PROC_PATH "/proc/tss"
#define BUF_SIZE 256 // Match the kernel buffer size

unsigned long extract_value(const char *buffer, const char *key)
{
	char *pos = strstr(buffer, key);
	if (pos) 
	{
		pos += strlen(key);
		return strtoul(pos, NULL, 10);
	}
	return 0;
}

int main()
{
	int fd;
	char buffer[BUF_SIZE];
	ssize_t bytes_read;
	unsigned long current_jiffies;
	char write_buf[64]; // Reduced size for writing

	printf("TSS Application: Reading from /proc/tss every second\n");
	printf("Press Ctrl+C to exit\n");

	while (1) 
	{
		// Open proc file
		fd = open(PROC_PATH, O_RDWR);
		if (fd < 0) 
		{
			perror("Error opening /proc/tss");
			sleep(1);
			continue;
		}

		// Read current values
		bytes_read = read(fd, buffer, BUF_SIZE - 1);
		if (bytes_read > 0) 
		{
			buffer[bytes_read] = '\0';

			// Extract my_jiffies value
			current_jiffies = extract_value(buffer, "my_jiffies: ");

			// Calculate new value (current_jiffies + 100)
			unsigned long new_jiffies = current_jiffies + 100;

			// Prepare write buffer
			snprintf(write_buf, sizeof(write_buf), "my_new_jiffies=%lu", new_jiffies);

			// Write back to position 0
			lseek(fd, 0, SEEK_SET);
			write(fd, write_buf, strlen(write_buf));

			printf("Read my_jiffies: %lu, Wrote my_new_jiffies: %lu\n", 
					current_jiffies, new_jiffies);
		}

		close(fd);
		sleep(1);
	}

	return 0;
}
