#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    char buf[1024];
    int fd;
    ssize_t r;
    fd = open("/dev/syncdev", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    r = read(fd, buf, sizeof(buf)-1);
    if (r < 0) perror("read");
    else {
        buf[r] = '\0';
        printf("read %zd bytes: '%s'\n", r, buf);
    }
    close(fd);
    return 0;
}
