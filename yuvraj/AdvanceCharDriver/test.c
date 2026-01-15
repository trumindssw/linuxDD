#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MYDRV_SET_TIMER_VAL  _IOW('a', 1, int)
#define MYDRV_ENABLE_TIMER  _IO('a', 2)
#define MYDRV_STOP_TIMER    _IO('a', 3)

int main()
{
    int fd, t = 2;

    fd = open("/dev/mydrv", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    ioctl(fd, MYDRV_SET_TIMER_VAL, &t);
    ioctl(fd, MYDRV_ENABLE_TIMER);

    sleep(5);

    ioctl(fd, MYDRV_STOP_TIMER);
    close(fd);

    return 0;
}
