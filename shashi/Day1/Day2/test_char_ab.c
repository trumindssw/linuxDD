// test_char_ab.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd;
    char ch, out;

    fd = open("/dev/char_ab", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    ch = 'a';
    write(fd, &ch, 1);
    read(fd, &out, 1);
    printf("Sent: %c, Got: %c\n", ch, out);

    ch = 'A';
    write(fd, &ch, 1);
    read(fd, &out, 1);
    printf("Sent: %c, Got: %c\n", ch, out);

    close(fd);
    return 0;
}

