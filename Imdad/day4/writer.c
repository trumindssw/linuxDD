#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char* msg = "Hello from writer!";
    int fd = open("/dev/syncdev", O_WRONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    if (argc > 1)
        msg = argv[1];

    ssize_t w = write(fd, msg, strlen(msg));
    if (w < 0) perror("write");
    else printf("written %zd bytes\n", w);
    close(fd);
    return 0;
}
