#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include "i2c_lib.h"

// I2C Linux device handle
static int g_fd;

// open the Linux device
int  i2cOpen(char *device)
{
	g_fd = open(device, O_RDWR);
	if (g_fd < 0) {
		perror("i2cOpen");
	    return -1;
	}

    return g_fd;
}

// close the Linux device
void i2cClose()
{
	close(g_fd);
}

// set the I2C slave address for all subsequent I2C device transfers
int  i2cSetAddress(int address)
{
	printf("Set i2c device address: %x\n", address);

	if (ioctl(g_fd, I2C_SLAVE, address) < 0) {
		perror("i2cSetAddress");
		return -1;
	}

    return 0;
}

// write a 8 bit value to a register pair
int  writeRegByte(uint8_t reg, uint8_t *value)
{
	uint8_t data[2];

    printf("I2C: write reg: 0x%02x, val: 0x%02x \n", reg, *value);

	data[0] = reg;
	data[1] = *value;
	if (write(g_fd, data, 2) != 2) {
		perror("writeRegByte");
        return -1;
	}

    return 0;
}

int  writeRegWord(uint8_t reg, uint16_t *value)
{
	uint8_t data[3];

    printf("I2C: write reg: 0x%02x, val: 0x%04x \n", reg, *value);

	data[0] = reg;
	data[1] = *value & 0xff;
	data[2] = (*value >> 8) & 0xff;
	if (write(g_fd, data, 3) != 2) {
		perror("writeRegWord");
        return -1;
	}

    return 0;
}

// read bytes from a register
int  readRegBytes(uint8_t reg, uint8_t *buf, int len)
{
	uint8_t data[1];

    printf("I2C: read reg: 0x%02x, len: %d \n", reg, len);

	data[0] = reg;
	if (write(g_fd, data, 1) != 1) {
		perror("set register");
		return -1;
	}

//	usleep(100000);
	if (read(g_fd, buf, len) <= 0) {
		perror("read value");
		return -1;
	}

	return len;
}

int  readRegByte(uint8_t reg, uint8_t *value)
{
    return readRegBytes(reg, value, 1);
}

int  readRegWord(uint8_t reg, uint16_t *value)
{
    return readRegBytes(reg, (uint8_t *)value, 2);
}

int  setRegBitState(uint8_t reg, uint8_t state, uint8_t bitMask)
{
    uint8_t val = 0;
    int ret;

    ret = readRegByte(reg, &val);
    if (ret < 0) {
        printf("%s: read error.\n", __func__);
        return -1;
    }

    if (state)
        val |= bitMask;
    else
        val &= ~bitMask;

    ret = writeRegByte(reg, &val);
    if (ret < 0) {
        printf("%s: write error.\n", __func__);
        return -1;
    }

    return 0;
}
