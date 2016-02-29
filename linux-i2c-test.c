// I2C test program 

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
#include <getopt.h>

#include "i2c_lib.h"

static char *deviceList[16] = {
    "/dev/i2c-0", "/dev/i2c-1", "/dev/i2c-2", "/dev/i2c-3"
};

static void usage (FILE * fp, int argc, char **argv)
{
    fprintf (fp,
       "Usage: %s [options]\n\n"
       "Options:\n"
       "-b | --bus           i2c bus number\n"
       "-d | --address       i2c slave device address\n"
       "-w | --word          read/write 16 bits value\n"
       "-r | --reg           register\n"
       "-h | --help          Print this message\n"
       "", argv[0]);
}
 
static const char short_options[] = "b:d:wr:h";
 
static const struct option long_options[] = {
    {"bus", required_argument, NULL, 'b'},
    {"address", required_argument, NULL, 'd'},
    {"word", no_argument, NULL, 'w'},
    {"reg", required_argument, NULL, 'r'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
};

int main(int argc, char** argv)
{
	int reg = 0;
	int value = -1;
	int addr = 0x00;
	uint8_t buf[16];
    int bus = 0;

	char *devName = "/dev/i2c-0";
	int word = 0;

	for (;;) {
	        int index;
        	int c;
 
	        c = getopt_long (argc, argv, short_options, long_options, &index);
 
        	if (-1 == c)
	            break;
        	switch (c) {
	        case 0:     /* getopt_long() flag */
        	    break;
	        case 'b':
        	    bus = strtol(optarg, NULL, 0);
	            break;
	        case 'd':
        	    addr = strtol(optarg, NULL, 0);
	            break;
	        case 'w':
        	    word = 1;
	            break;
        	case 'r':
	            reg = strtol(optarg, NULL, 0);
        	    break;
        	case 'h':
	        default:
        	    usage (stderr, argc, argv);
	            exit (EXIT_FAILURE);
        	}
	}

    if (optind < argc)
        value = strtol(argv[optind], NULL, 0);

    if ((bus >= 0) && (bus < 4))
        devName = deviceList[bus];
	// open Linux I2C device
	i2cOpen(devName);

	// set address of the PCA9555
	i2cSetAddress(addr);

	memset(buf, 0, 16);
    if (word) {
        if (value != -1)
            writeRegWord(reg, (uint16_t *)&value);
        readRegWord(reg, (uint16_t *)buf);
	    printf("reg = 0x%02x, value = %04x\n", reg, *((uint16_t *)buf));
    }
    else {
        if (value != -1)
            writeRegByte(reg, (uint8_t *)&value);
        readRegByte(reg, buf);
	    printf("reg = 0x%02x, value = %02x\n", reg, buf[0]);
    }

	// close Linux I2C device
	i2cClose();

	return 0;
}
