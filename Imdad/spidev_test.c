#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>       
#include <unistd.h>      
#include <sys/ioctl.h>   // For ioctl()
#include <linux/spi/spidev.h>  // SPI ioctl macros 

#define SPI_MODE        SPI_MODE_0   //(CPOL = 0, CPHA = 0)
#define BITS_PER_WORD   8            // 8 bits per word
#define SPEED           500000       // 500 kHz SPI clock speed

int main(int argc, char *argv[]) {
    int fd;
    const char *device;
    uint8_t mode;
    uint8_t bits;
    uint32_t speed;
    int ret;

    device = "/dev/spidev3.0";
    mode = SPI_MODE;
    bits = BITS_PER_WORD;
    speed = SPEED;
    
    uint8_t tx[] = "Hello";         // Transmit buffer
    uint8_t rx[sizeof(tx)] = {0};   // Receive buffer (initialized to 0)

    if (argc > 1)
        device = argv[1];

    // Open the SPI device in read/write mode
    fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("open"); 
        return 1;
    }

    // Set SPI mode (e.g., mode 0)
    ioctl(fd, SPI_IOC_WR_MODE, &mode);

    // Set number of bits per word (typically 8)
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);

    // Set SPI clock speed in Hz
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // Define SPI transfer structure
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,       
        .rx_buf = (unsigned long)rx,       
        .len = sizeof(tx),                 
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    // SPI_IOC_MESSAGE(1) means we're sending 1 spi_ioc_transfer struct
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("SPI_IOC_MESSAGE"); 
        return 1;
    }

    // Print received data in hex format
    printf("Received:\n");
    for (int i = 0; i < sizeof(rx); i++)
        printf("0x%02X ", rx[i]);
    printf("\n");

    // Close the SPI device file descriptor
    close(fd);
    return 0;
}
