#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MYDRV_MAGIC         'T'
#define MYDRV_ENABLE_TIMER  _IO(MYDRV_MAGIC, 1)
#define MYDRV_STOP_TIMER    _IO(MYDRV_MAGIC, 2)
#define MYDRV_SET_TIMER_VAL _IOW(MYDRV_MAGIC, 3, int)

int main() {
    int fd = open("/dev/mydrv", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    int t = 2;
    ioctl(fd, MYDRV_SET_TIMER_VAL, &t);

    for (int i=0; i<5; i++) {
        ioctl(fd, MYDRV_ENABLE_TIMER);
        sleep(10);   // trigger every 10 sec
    }

    ioctl(fd, MYDRV_STOP_TIMER);
    close(fd);
    return 0;
}

