// user.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_WRITE_DATETIME _IO('a', 1)

int main() {
    int fd = open("/dev/mydev", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    ioctl(fd, IOCTL_WRITE_DATETIME);
    close(fd);

    printf("Datetime written. Check /tmp/datetime.txt\n");
    return 0;
}
