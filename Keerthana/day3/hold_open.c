#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("/dev/semmutex0", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    printf("opened fd=%d, sleeping 60s\n", fd);
    sleep(60);  // hold the device open
    close(fd);
    printf("closed fd\n");
    return 0;
}

