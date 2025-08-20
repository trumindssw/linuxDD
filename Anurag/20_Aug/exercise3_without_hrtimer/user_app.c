// UserApp

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define DEVICE "/dev/my_drv"

struct ioctl_arg {
    char path[256];
    int tz_offset;   // using it to UTC -> IST
    int setval;
};

#define MYDRV_ENABLE_TIMER  _IO('M', 1)
#define MYDRV_SET_TIMER_VAL _IOW('M', 2, unsigned int)
#define MYDRV_STOP_TIMER    _IO('M', 3)

int main()
{
    int fd;
    int i;
    struct ioctl_arg arg;
    
    time_t now = time(NULL);
    struct tm local_tm = *localtime(&now);
    struct tm gm_tm = *gmtime(&now);


    strcpy(arg.path, "/tmp/datetime.txt");
    arg.setval = 5;
    // compute offset (local - UTC) in seconds
    arg.tz_offset = (local_tm.tm_hour - gm_tm.tm_hour) * 3600 +
                    (local_tm.tm_min - gm_tm.tm_min) * 60;

    // handle day wraparound
    if (local_tm.tm_yday != gm_tm.tm_yday) {
        if (local_tm.tm_yday > gm_tm.tm_yday)
            arg.tz_offset += 24 * 3600;
        else
            arg.tz_offset -= 24 * 3600;
    }

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open device");
        return 1; 
    }

    printf("Setting timer value to 5s\n");

    if (ioctl(fd, MYDRV_SET_TIMER_VAL, &arg) < 0) {
        perror("ioctl SET_TIMER_VAL");
        close(fd);
        return 1;
    }

    printf("Enabling timer via IOCTL\n");

    if (ioctl(fd, MYDRV_ENABLE_TIMER) < 0) {
        perror("ioctl ENABLE");
        return 1;
    }

    close(fd);

    printf("Writing done into file /tmp/datetime.txt");
    
    return 0;
}
