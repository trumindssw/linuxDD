#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

int main() {
    int uart_fd;
    struct termios tty; // structure that carry uart configuration, here ttyACM0
    char *device = "/dev/ttyACM0";   // Rb3 tty node  

    // Open UART device
    uart_fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (uart_fd < 0) {
        perror("Error opening UART");
        return -1;
    }

    // Get current UART config
    if (tcgetattr(uart_fd, &tty) != 0) {
        perror("tcgetattr error");
        close(uart_fd);
        return -1;
    }

    // Configure UART
    cfsetospeed(&tty, B9600);   // Set baud rate to 115200
    cfsetispeed(&tty, B115200);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  // 8-bit chars
    tty.c_iflag &= ~IGNBRK;  // Disable break processing
    tty.c_lflag = 0;         // No signaling chars, no echo
    tty.c_oflag = 0;         // No remapping
    tty.c_cc[VMIN]  = 10;     // Read blocks until at least 10 char arrives
    tty.c_cc[VTIME] = 1;     // Timeout 1 sec
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No flow control
    tty.c_cflag |= (CLOCAL | CREAD);        // Enable receiver
    tty.c_cflag &= ~(PARENB | PARODD);      // No parity
    tty.c_cflag &= ~CSTOPB;                 // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                // No RTS/CTS

    // Set current UART config
    if (tcsetattr(uart_fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr error");
        close(uart_fd);
        return -1;
    }

    // Write data
    const char *msg = "cd /proc \n";
    write(uart_fd, msg, strlen(msg));

    // Read data
    char buf[100];
    int n = read(uart_fd, buf, sizeof(buf));
    if (n > 0) {
        buf[n] = '\0';
        printf("Received: %s\n", buf);
    }

    close(uart_fd);
    return 0;
}

