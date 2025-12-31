#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(void)
{
    int fd;
    char buf[32];

    fd = open("/dev/semmutex0", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* write example */
    write(fd, "Hello", 5);

    lseek(fd, 0, SEEK_SET);
    memset(buf, 0, sizeof(buf));
    read(fd, buf, 32);
    printf("read: '%s'\n", buf);

    close(fd);
    return 0;
}

