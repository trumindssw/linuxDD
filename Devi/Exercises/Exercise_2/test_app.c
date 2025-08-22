#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#define SET_TIMER_VAL _IOW('a', 'a', int32_t*)
#define ENABLE_TIMER _IO('a', 'b')
#define STOP_TIMER _IO('a', 'c')

// Function to test file concatenation while spinlock is held
void test_file_concat() {
	printf("Testing file concatenation while spinlock is held...\n");

	// Try to write to the file multiple times to see if it gets blocked
	for (int i = 0; i < 5; i++) {
		int result = system("echo 'User mode text' >> /tmp/timer_log.txt");
		if (result == 0) {
			printf("Successfully wrote to file at attempt %d\n", i + 1);
		} else {
			printf("Failed to write to file at attempt %d (might be blocked by spinlock)\n", i + 1);
		}
		sleep(1);
	}
}

int main() {
	int fd;
	int timer_value;
	char choice;

	printf("Timer Driver Test Application\n");

	fd = open("/dev/timer_driver", O_RDWR);
	if (fd < 0) {
		perror("Failed to open device");
		printf("Please make sure the timer_driver module is loaded and device node exists\n");
		return -1;
	}

	while (1) {
		printf("\nOptions:\n");
		printf("1. Set timer value\n");
		printf("2. Enable timer (every 10 sec)\n");
		printf("3. Stop timer\n");
		printf("4. Test file concatenation\n");
		printf("5. View log file\n");
		printf("6. Exit\n");
		printf("Choice: ");
		scanf(" %c", &choice);

		switch (choice) {
			case '1':
				printf("Enter timer wait time (seconds): ");
				scanf("%d", &timer_value);
				if (ioctl(fd, SET_TIMER_VAL, &timer_value) < 0) {
					perror("SET_TIMER_VAL failed");
				} else {
					printf("Timer value set to %d seconds\n", timer_value);
				}
				break;

			case '2':
				if (ioctl(fd, ENABLE_TIMER) < 0) {
					perror("ENABLE_TIMER failed");
				} else {
					printf("Timer enabled - will trigger every 10 seconds\n");
					printf("The workqueue will:\n");
					printf("  1. Acquire spinlock\n");
					printf("  2. Write timestamp to /tmp/timer_log.txt\n");
					printf("  3. Wait for %d seconds\n", timer_value);
					printf("  4. Release spinlock\n");
				}
				break;

			case '3':
				if (ioctl(fd, STOP_TIMER) < 0) {
					perror("STOP_TIMER failed");
				} else {
					printf("Timer stopped\n");
				}
				break;

			case '4':
				test_file_concat();
				break;

			case '5':
				printf("Contents of /tmp/timer_log.txt:\n");
				system("cat /tmp/timer_log.txt 2>/dev/null || echo 'File does not exist yet'");
				break;

			case '6':
				close(fd);
				printf("Exiting...\n");
				return 0;

			default:
				printf("Invalid choice\n");
		}

		// Clear input buffer
		while (getchar() != '\n');
	}

	close(fd);
	return 0;
}

