#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>

#define DEVICE "/dev/datetime_drv"
#define IOCTL_WRITE_DATETIME _IOW('d', 1, struct ioctl_arg *)

struct ioctl_arg {
    char path[256];
    int tz_offset;   // seconds from UTC
};

int main() {
    int fd;
    struct ioctl_arg arg;
    time_t now = time(NULL);
    struct tm local_tm = *localtime(&now);
    struct tm gm_tm = *gmtime(&now);    

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

    strcpy(arg.path, "./datetime.txt");

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    if (ioctl(fd, IOCTL_WRITE_DATETIME, &arg) == -1) {
        perror("ioctl failed");
        close(fd);
        return 1;
    }

    printf("Datetime written to datetime.txt (local time)\n");

    close(fd);
    return 0;
}
