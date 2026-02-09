#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd;
    char write_buf[] = "Hello Kernel!";
    char read_buf[1024] = {0};

    fd = open("/dev/simplechar", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    write(fd, write_buf, strlen(write_buf));
    read(fd, read_buf, sizeof(read_buf));
    printf("Read from device: %s\n", read_buf);

    close(fd);
    return 0;
}
