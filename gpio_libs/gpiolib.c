//
// (c) 2016 bqin, All Rights Reserved.
//
// The Linux implementation relies on the kernel's GPIOLIB interface
//  to access the GPIO pins from the User Space.
// To enable the GPIOLIB interface make sure you compile your kernel with the following options.
//
//  CONFIG_ARCH_REQUIRE_GPIOLIB
//  CONFIG_GPIOLIB
//  CONFIG_GPIO_SYSFS
//
// Check that /sys/class/gpio exists.
// Now you can access and manipulate the GPIO's from User Space using the standard C calls
// such as Open, Write, Read, Close etc.
//

#include <stdio.h>
#include <stdlib.h>    // atoi
#include <string.h>    // memset
#include <sys/stat.h>  // open
#include <fcntl.h>     // open
#include <unistd.h>    // close, write
#include <stdbool.h>
#include <poll.h>
#include <errno.h>
#include <stdbool.h>

#include "gpiolib.h"

#define GPIO_DIRECTION_INPUT    0
#define GPIO_DIRECTION_OUTPUT   1

typedef struct
{
    int  fd;
    int  direction;
    gpio_interrupt_t interrupt;
} gpioStatusArray_t;

#define MAX_GPIO_PINS   128
static gpioStatusArray_t gpioStatusArray[MAX_GPIO_PINS];
static bool gpioInitialized = false;
static bool gpioShuttingDown = false;

void gpio_init(void)
{
    int i;
    if (gpioShuttingDown)   // must be program shutdown.
        return;

    for (i = 0; i < MAX_GPIO_PINS; i++)
    {
        gpioStatusArray[i].fd         = -1;
        gpioStatusArray[i].direction  = GPIO_DIRECTION_INPUT;
        gpioStatusArray[i].interrupt  = Gpio_INTERRUPT_NONE; 
    }
    gpioInitialized = true;
}

//----------------------------------------------------------------------
void gpio_term(void)
{
    int i;
    gpioShuttingDown = true;
    for (i = 0; i < MAX_GPIO_PINS; i++)
    {
        if (gpioStatusArray[i].fd != -1)
        {
            close(gpioStatusArray[i].fd);
            gpioStatusArray[i].fd = -1;
        }
        gpioStatusArray[i].direction  = GPIO_DIRECTION_INPUT;
        gpioStatusArray[i].interrupt  = Gpio_INTERRUPT_NONE; 
    }
    gpioShuttingDown = false;
    gpioInitialized = false;
}

// Local utility function
static int gpio_setSysFS_state(int num, int direction, gpio_interrupt_t interrupt)
{
    int  fd;
    char nameString[128];
    bool justOpenedFd = false;

    if (!gpioInitialized) 
        gpio_init();

    // open for export, if not already opened.
    if (gpioStatusArray[num].fd == -1)
    {
        justOpenedFd = true;
        // To start, the specific GPIO pin has to be exported and configured appropriately.
        fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd < 0)
        {
            printf("gpio_setSysFS_state returns NO_EXPORT.\n");
            return -1;
        }
        // Each GPIO on the Processor will have a unique number, please refer to the processor manual.
        // Write the GPIO you want to export and close the export interface.
        sprintf(nameString, "%d", num);
        write(fd, nameString, 4);
        close(fd);
    }
    if (justOpenedFd || (direction != gpioStatusArray[num].direction)) 
    {  // need to write the dir file.
        sprintf(nameString, "/sys/class/gpio/gpio%d/direction", num);
        fd = open(nameString, O_RDWR);
        if (fd < 0)
        {
            printf("gpio_setSysFS_state returns NO_DIRECTION.\n");
            return -1;
        }
        if (direction == GPIO_DIRECTION_OUTPUT) 
            write(fd, "out", 4);
        else
            write(fd, "in", 3);
        close(fd);
        gpioStatusArray[num].direction = direction;
    }
    if (justOpenedFd) 
    {
        sprintf(nameString, "/sys/class/gpio/gpio%d/value", num);
        fd = open(nameString, O_RDWR);
        if (fd < 0)
        {
            printf("gpio_setSysFS_state returns NO_VALUE.\n");
            return -1;
        }
        // leave this one open.
        gpioStatusArray[num].fd = fd;
    }
    if (justOpenedFd || (gpioStatusArray[num].interrupt != interrupt))
    {
        sprintf(nameString, "/sys/class/gpio/gpio%d/edge", num);
        fd = open(nameString, O_RDWR);
        if (fd < 0)
        {
            printf("gpio_setSysFS_state returns NO_EDGE.\n");
            return -1;
        }
        switch (interrupt) 
        {
        default:
        case Gpio_INTERRUPT_NONE:
            // printf("interrupt set to none.");
            write(fd, "none", 5);
            break;
        case Gpio_INTERRUPT_FALLING_EDGE:
            // printf("interrupt set to falling.");
            write(fd, "falling", 8);
            break;
        case Gpio_INTERRUPT_RISING_EDGE:
            // printf("interrupt set to rising.");
            write(fd, "rising", 7);
            break;
        case Gpio_INTERRUPT_BOTH_EDGES:
            // printf("interrupt set to both.");
            write(fd, "both", 5);
            break;
        }
        close(fd);
        gpioStatusArray[num].interrupt = interrupt;
    }
    return 0;
}


//----------------------------------------------------------------------
int gpio_set_value(int num, int val)
{
    int rval;

    if (gpioShuttingDown)
        return 0;

    rval = gpio_setSysFS_state(num, GPIO_DIRECTION_OUTPUT, Gpio_INTERRUPT_NONE);
    if (rval != 0) 
    {
        printf("%s: Gpio_setSysFS_state returns 0x%x.\n", __func__, rval);
        return rval;
    }

    lseek(gpioStatusArray[num].fd, 0, SEEK_SET);
    if (val == 0)
    {
        // printf("Writing 0 to pin %d\n", num);
        write(gpioStatusArray[num].fd, "0", 2);  // To make the GPIO line low
    } else
    {
        // printf("Writing 1 to pin %d\n", num);
        write(gpioStatusArray[num].fd, "1", 2);  // To make the GPIO line low
    }
    // printf("Used fd %d to write %d from pin %d\n.", gpioStatusArray[num].fd, val, num);
    return 0;
}

//----------------------------------------------------------------------
int gpio_get_value(int num)
{
    char buf[8];
    int rval = 0;

    if (gpioShuttingDown)
        return 0;

    rval = gpio_setSysFS_state(num, GPIO_DIRECTION_INPUT, Gpio_INTERRUPT_NONE);
    if (rval != 0) 
    {
        printf("%s: gpio_setSysFS_state returns 0x%x.\n", __func__, rval);
        return rval;
    }

    lseek(gpioStatusArray[num].fd, 0, SEEK_SET);  // rewind
    rval = read(gpioStatusArray[num].fd, buf, 2);
    // printf("read %c, %d\n", buf[0], (int)buf[0]);

    if ((int)buf[0] == 48) 
    {
        // printf("Used fd %d to read 0 from pin %d with rval %d.\n", gpioStatusArray[num].fd, num, rval);
        return 0;
    }
    else 
    {
        // printf("Used fd %d to read 1 from pin %d with rval %d.\n", gpioStatusArray[num].fd, num, rval);
        return 1;
    }
}

//----------------------------------------------------------------------
// timeout in milliseconds
int gpio_wait_edge(int num, gpio_interrupt_t edge, int timeout, int *val)
{
    int rval = 0;
    struct pollfd fdset[1];
    char buf[8];

    if (gpioShuttingDown)
        return 0;

    rval = gpio_setSysFS_state(num, GPIO_DIRECTION_INPUT, edge);
    if (rval != 0) 
    {
        printf("%s: gpio_setSysFS_state returns 0x%x.\n", __func__, rval);
        return rval;
    }

    memset((void*)fdset, 0, sizeof(fdset));

    fdset[0].fd     = gpioStatusArray[num].fd;
    fdset[0].events = POLLPRI;

    rval = poll(fdset, 1, timeout);

    if (rval < 0) {
        // printf("gpio_waitForEdge: poll() failed with %d!\n", rval);
        return -3;
    }
    else if (rval == 0) {
        // printf("gpio_waitForEdge: poll() returned.\n");
        return -1;
    }
    // else printf("gpio_waitForEdge: poll() returned %d.\n", rval);
                
    lseek(gpioStatusArray[num].fd, 0, SEEK_SET);  // rewind
    read(fdset[0].fd, buf, 2);
    // printf("gpio_waitForEdge:  GPIO %d interrupt occurred\n", num);
    if ((int)buf[0] == 48) 
        *val = 0;
    else 
        *val = 1;

    return 0;
}


//----------------------------------------------------------------------
//  Sometimes you must wait for multiple things.
//   You can wait for an edge of one of several GPIO pins.
//  Sometimes you need to interrupt the waiting.
//   You can also wait on a file descriptor.  Writing to this will trigger a return here.
//
//  To avoid dynamic allocation of memory, you can wait on no more than 4 things.
//
int gpio_wait_edge_multi(int *pins,     // in:  an array of pins on which to wait.
                             gpio_interrupt_t *edges, // in:  a null terminated array of edges matching pins.
                             int fd,        // in:  an optional open file descriptor to wait on additionally.  -1 to ignore.
                             int timeout,   // in:  timeout in milliseconds
                             int *pin,      // out:  which pin triggered.  Negative for fd.
                             int *val)      // out:  value of pin after trigger
{
    int rval = 0;
    struct pollfd fdset[GPIO_MAX_MULTIPLE_WAIT_SOURCES+1];  // +1 in case of fd.
    int i, nfds, numPins;
    char buf[8];

    if (gpioShuttingDown)
        return 0;

    // initial check.  Return errors before doing anything.
    for (i = 0; i < GPIO_MAX_MULTIPLE_WAIT_SOURCES; i++)
    {
        if (edges[i] == (int)NULL)
            break;  // end of the null terminated array.
    }
    if (i >= GPIO_MAX_MULTIPLE_WAIT_SOURCES)
    {
        printf("%s: If you need more pins, change constant.\n", __func__);
        return -2;
    }

    *pin = *val = 0;
    memset((void*)fdset, 0, sizeof(fdset));

    for (i = 0; i < GPIO_MAX_MULTIPLE_WAIT_SOURCES; i++)
    {
        if (edges[i] == (int)NULL)
            break;  // end of the null terminated array.
        rval = gpio_setSysFS_state(pins[i], GPIO_DIRECTION_INPUT, edges[i]);
        if (rval != 0) 
        {
            printf("%s: gpio_setSysFS_state returns 0x%x on entry %d.\n", __func__, rval, i);
            return rval;
        }
        fdset[i].fd = gpioStatusArray[pins[i]].fd;
        fdset[i].events = POLLPRI;
    }
    // printf("gpio_waitForEdgeMulti: setting up to wait on %d pins\n", i);
    nfds = numPins = i;
    if (fd > 0)
    {
        nfds++;
        fdset[i].fd = fd;
        fdset[i].events = POLLIN;
        // printf("gpio_waitForEdgeMulti: setting up to wait on file descriptor %d.\n", fd);
    }

    // actually wait
    rval = poll(fdset, nfds, timeout);

    // handle other cases.
    if (rval < 0) {
        printf("%s: poll() failed with %d!\n", __func__, rval);
        return -3;
    }
    else if (rval == 0) {
        printf("%s: poll() returned timeout.\n", __func__);
        return -1;
    }

    // which thing occurred?
    for (i=0; i<nfds; i++)
    {
        if ( (fdset[i].events == POLLPRI) &&
             (fdset[i].revents & POLLPRI)) 
        {   // this pin triggered;
            lseek(fdset[i].fd, 0, SEEK_SET);  // rewind
            read (fdset[i].fd, buf, 2);
            *pin = pins[i];
            if ((int)buf[0] == 48) 
                *val = 0;
            else 
                *val = 1;

            return 0;
        }

        if (fdset[i].revents & POLLIN) 
        {   // Check the file
            (void)read(fdset[i].fd, buf, 1);
            *pin = -1;

            return 0;
        }
    }

    printf("gpio_waitForEdge found no cause!\n");
    return -3;
}

// when a thread is waiting on the GPIO line, use this to peek at the 
// value without resetting the state.
// Will return error if pin is not waiting.
int gpio_peek(int num, int *val)
{
    char buf[8];

    if (gpioShuttingDown)
        return 0;

    if (gpioStatusArray[num].fd == -1)
        return -1;

    if (gpioStatusArray[num].direction != GPIO_DIRECTION_INPUT)
        return -1;

    if (gpioStatusArray[num].interrupt == Gpio_INTERRUPT_NONE)
        return -1;

    lseek(gpioStatusArray[num].fd, 0, SEEK_SET);  // rewind
    read (gpioStatusArray[num].fd, buf, 2);

    if ((int)buf[0] == 48) 
        *val = 0;
    else 
        *val = 1;

    return 0;
}


