#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MY_MAGIC 'T'
#define IOCTL_TIMER_SET   _IOW(MY_MAGIC, 1, int)
#define IOCTL_TIMER_START _IO(MY_MAGIC, 2)
#define IOCTL_TIMER_STOP  _IO(MY_MAGIC, 3)

int main() {
    int fd = open("/dev/mytimer", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    int interval = 10; // 10 ms
    ioctl(fd, IOCTL_TIMER_SET, &interval);
    ioctl(fd, IOCTL_TIMER_START);

    printf("Timer started with %d ms interval. Check /tmp/timer_log.txt\n", interval);

    sleep(2); // sleep for a while

    ioctl(fd, IOCTL_TIMER_STOP);
    close(fd);
    return 0;
}

