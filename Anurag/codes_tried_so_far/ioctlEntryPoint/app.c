#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "mydrv_ioctl.h"

int main()
{
    int fd;
    int timer_val = 3;

    fd = open("/dev/my_drv", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/my_drv");
        return -1;
    }

    printf("Enabling timer...\n");
    ioctl(fd, MYDRV_ENABLE_TIMER);

    printf("Setting timer value = %d\n", timer_val);
    ioctl(fd, MYDRV_SET_TIMER_VAL, &timer_val);

    printf("Sleeping 10 seconds to observe timer...\n");
    sleep(10);

    printf("Stopping timer...\n");
    ioctl(fd, MYDRV_STOP_TIMER);

    close(fd);
    return 0;
}
