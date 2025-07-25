#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_GET_VALUE _IOR('i', 1, int *)
#define IOCTL_SET_VALUE _IOW('i', 2, int *)

int main() {
    int fd = open("/dev/ioctl_debug", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int val = 0;

    // Get value from kernel
    ioctl(fd, IOCTL_GET_VALUE, &val);
    printf("Got from kernel: %d\n", val);

    // Set new value
    val = 99;
    ioctl(fd, IOCTL_SET_VALUE, &val);
    printf("Sent to kernel: %d\n", val);

    // Confirm change
    ioctl(fd, IOCTL_GET_VALUE, &val);
    printf("Got again: %d\n", val);

    close(fd);
    return 0;
}
