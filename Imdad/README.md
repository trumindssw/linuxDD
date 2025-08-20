simple C program to test SPI full-duplex communication on a Linux system using the spidev interface.

build for ARM64:
aarch64-linux-gnu-gcc spi_test.c -o spi_test

Run on specific device:
./spidev_test /dev/spidev3.0

The program prints received bytes (in hex) from the SPI slave. If loopback is enabled (MOSI connected to MISO), it should return the same data else it will return all zeroes.
