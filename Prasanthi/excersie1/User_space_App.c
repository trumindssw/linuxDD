#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MYDRV_MAGIC         'T'
#define MYDRV_ENABLE_TIMER  _IO(MYDRV_MAGIC, 1)
#define MYDRV_STOP_TIMER    _IO(MYDRV_MAGIC, 2)
#define MYDRV_SET_TIMER_VAL _IOW(MYDRV_MAGIC, 3, int)

int main()
{
    int fd, val, choice;

    fd = open("/dev/mydrv", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    printf("1. Set Timer Value\n");
    printf("2. Enable Timer\n");
    printf("3. Stop Timer\n");
    printf("4. Exit\n");

    while (1) {
        printf("\nEnter choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            printf("Enter seconds (1 or 2): ");
            scanf("%d", &val);
            ioctl(fd, MYDRV_SET_TIMER_VAL, &val);
            break;

        case 2:
            ioctl(fd, MYDRV_ENABLE_TIMER);
            printf("Sent ENABLE_TIMER\n");
            break;

        case 3:
            ioctl(fd, MYDRV_STOP_TIMER);
            printf("Sent STOP_TIMER\n");
            break;

        case 4:
            close(fd);
            return 0;

        default:
            printf("Invalid choice\n");
        }
    }
}

