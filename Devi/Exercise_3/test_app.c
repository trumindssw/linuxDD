#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define MAGIC 'T'
#define SET_TIMER_VAL _IOW(MAGIC, 1, int)
#define ENABLE_TIMER _IO(MAGIC, 2)
#define STOP_TIMER _IO(MAGIC, 3)

int main(int argc, char *argv[])
{
	int fd;
	int timer_val;
	char choice;

	fd = open("/dev/timer_dev", O_RDWR);
	if (fd < 0) 
	{
		perror("Failed to open device");
		return -1;
	}

	printf("=== Timer Driver Test Application ===\n");
	printf("Commands:\n");
	printf("  s <ms> - Set timer value (100-5000 ms)\n");
	printf("  e      - Enable timer (trigger workqueue)\n");
	printf("  t      - Stop timer (ignore future requests)\n");
	printf("  q      - Quit application\n");
	printf("  d      - Display kernel messages (sudo dmesg | grep TIMER_LOG)\n");

	while (1) 
	{
		printf("\n> ");
		scanf(" %c", &choice);

		switch (choice) 
		{
			case 's':
				printf("Enter timer value (100-5000 ms): ");
				scanf("%d", &timer_val);
				if (ioctl(fd, SET_TIMER_VAL, &timer_val) < 0) 
				{
					perror("SET_TIMER_VAL failed");
				} 
				else 
				{
					printf("Timer value set to %d ms\n", timer_val);
				}
				break;

			case 'e':
				if (ioctl(fd, ENABLE_TIMER) < 0) 
				{
					perror("ENABLE_TIMER failed");
				} 
				else 
				{
					printf("Timer enabled - check kernel messages\n");
				}
				break;

			case 't':
				if (ioctl(fd, STOP_TIMER) < 0) 
				{
					perror("STOP_TIMER failed");
				} 
				else 
				{
					printf("Timer stopped\n");
				}
				break;

			case 'd':
				printf("Run: sudo dmesg | grep TIMER_LOG\n");
				break;

			case 'q':
				close(fd);
				return 0;

			default:
				printf("Invalid command\n");
				break;
		}
	}

	close(fd);
	return 0;
}

