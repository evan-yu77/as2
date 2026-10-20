#include "hal/adc.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SPI_DEV "/dev/spidev0.0"

// variables for SPI implementation

static int spi_fd = -1; // file descriptor for SPI device
static uint8_t mode = SPI_MODE_0; // SPI mode
static uint8_t bits = 8; // bits per word
static uint32_t speed = 1000000; // max speed in Hz

void ADC_init(void) {
    spi_fd = open(SPI_DEV, O_RDWR);
    if (spi_fd < 0) {
        perror("cannot open SPI device; please try again");
        exit(EXIT_FAILURE);
    }

    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
}

int ADC_getRawReading(void) {
    if (spi_fd < 0) {
        fprintf(stderr, "SPI device not initialized\n");
        return -1;
    }

    uint8_t tx[] = {0x01, 0x80, 0x00}; // Start bit, single-ended mode, channel 0
    uint8_t rx[3] = {0}; // receive buffer

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 3,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    // Perform SPI transaction

    int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("SPI transaction failed");
        return -1;
    }

    // Combine the received bytes to get the 10-bit ADC value
    int adc_value = ((rx[1] & 0x03) << 8 | rx[2]);
    return adc_value;

}

void ADC_cleanup(void) {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
}

double ADC_getVoltageReading(void) {
    int raw_val = ADC_getRawReading();

    return ((double)raw_val / 1023.00) * 3.3; // using 3.3V ref voltage
    
}

