#include <stdio.h>  
#include <stdlib.h>
#include <stdint.h>
#include <time.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <unistd.h>   
#include <sys/mman.h>
#include <getopt.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

static void usage (FILE * fp, int argc, char **argv)
{
    fprintf (fp,
       "Usage: %s [options] reg_addr [value]\n\n"
       "Options:\n"
       "-w | --write         write register\n"
       "-h | --help          Print this message\n"
       "", argv[0]);
}

static const char short_options[] = "wh";

static const struct option long_options[] = {
    {"write", no_argument, NULL, 'w'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)  
{
    int dev_fd;
    off_t map_base;
    int wflag = 0;
    uint32_t reg = 0;
    uint32_t val = 0;
    unsigned int *reg_addr = NULL;

    for (;;) {
            int index;
            int c;

            c = getopt_long (argc, argv, short_options, long_options, &index);

            if (-1 == c)
                break;
            switch (c) {
            case 'w':
                wflag = 1;
                break;
            case 'h':
                usage (stdout, argc, argv);
                exit (EXIT_SUCCESS);
                break;
            default:
                usage (stderr, argc, argv);
                exit (EXIT_FAILURE);
            }
    }

    if (optind < argc)
    {
        reg = strtoul(argv[optind], NULL, 0);
    }
    else
    {
        usage (stderr, argc, argv);
        exit (EXIT_FAILURE);
    }

    if (wflag && ((optind + 1) >= argc))
    {
        usage (stderr, argc, argv);
        exit (EXIT_FAILURE);
    }
    else if ((optind + 1) < argc)
    {
        val = strtoul(argv[optind + 1], NULL, 0);
    }
    
    dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0)    
    {  
        printf("open(/dev/mem) failed.");      
        return 0;  
    }    

    map_base=(int)mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd,  (reg & ~MAP_MASK) );
    if(map_base == -1) 
    {
        printf( "%s.%d:  ERROR: mmap returned 0x%x.\n", __FUNCTION__, __LINE__, (int)map_base);
        close(dev_fd);
        return -1;
    }

    reg_addr = (unsigned int*)(map_base + (reg & MAP_MASK));

    if (wflag)
    {
        reg_addr[0] = val;
        printf("Write reg: addr = 0x%08x, val = 0x%08x, reg = 0x%08x\n", (int)reg_addr, val, reg_addr[0]);
    }
    else
    {
        printf("Read reg: addr = 0x%08x, val = 0x%08x\n", reg, reg_addr[0]);
    
    }

    if(dev_fd)  
        close(dev_fd);  

    munmap(reg_addr,MAP_SIZE);

    return 0;  
}  
