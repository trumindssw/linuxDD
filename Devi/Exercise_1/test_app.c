// test_app.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

// IOCTL commands
#define SET_TIMER_VAL _IOW('T', 1, int)
#define ENABLE_TIMER _IO('T', 2)
#define STOP_TIMER _IO('T', 3)

int main() 
{
	int fd;
	int choice;
	int timer_val;

	printf("Timer Driver Test Application\n");
	printf("=============================\n");

	// Open device
	fd = open("/dev/timer_device", O_RDWR);
	if (fd < 0) 
	{
		perror("Failed to open device");
		return 1;
	}

	while (1) 
	{
		printf("\nOptions:\n");
		printf("1. Set timer interval (1 or 2 seconds)\n");
		printf("2. Enable timer (triggers every 10 seconds)\n");
		printf("3. Stop timer\n");
		printf("4. Try to write to file during critical section\n");
		printf("5. Exit\n");
		printf("Enter choice: ");

		scanf("%d", &choice);

		switch (choice) 
		{
			case 1:
				printf("Enter timer interval (1 or 2): ");
				scanf("%d", &timer_val);
				if (ioctl(fd, SET_TIMER_VAL, timer_val) == 0) 
				{
					printf("Timer interval set to %d seconds\n", timer_val);
				} 
				else 
				{
					perror("IOCTL failed");
				}
				break;

			case 2:
				if (ioctl(fd, ENABLE_TIMER) == 0) 
				{
					printf("Timer enabled - will trigger every 10 seconds\n");
				} 
				else 
				{
					perror("IOCTL failed");
				}
				break;

			case 3:
				if (ioctl(fd, STOP_TIMER) == 0) 
				{
					printf("Timer stopped\n");
				} 
				else 
				{
					perror("IOCTL failed");
				}
				break;

			case 4:
				printf("Attempting to write to file during kernel's critical section...\n");
				system("echo 'User write attempt - ' $(date) >> /tmp/timer_driver.log");
				printf("Write attempted. Check if it was blocked by spinlock.\n");
				break;

			case 5:
				close(fd);
				printf("Exiting...\n");
				return 0;

			default:
				printf("Invalid choice\n");
		}
	}

	close(fd);
	return 0;
}
