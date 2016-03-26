/*
 *  Copyright (c) 2008-2016 bqin, buaaqbh@gmail.com.
 * All Rights Reserved.
 *
 * This source code and any compilation or derivative thereof is the
 * proprietary information of YinMan
 * and is confidential in nature.
 */

#ifndef __GPIOLIB_H_
#define __GPIOLIB_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef enum
{
    Gpio_INTERRUPT_NONE,
    Gpio_INTERRUPT_FALLING_EDGE,
    Gpio_INTERRUPT_RISING_EDGE,
    Gpio_INTERRUPT_BOTH_EDGES
} gpio_interrupt_t;

// auto initialized on first call to read or write.
void gpio_init(void);

// closes any sysFS file descriptors left open.
void gpio_term(void);

// Sets GPIO pin specified by number to value (zero or one).
// returns zero if succeeds.
int gpio_set_value(int num, int val);

// Reads GPIO pin specified by number and returns zero or 1.
int gpio_get_value(int num);

//----------------------------------------------------------------------
// timeout in milliseconds
int gpio_wait_edge(int num, gpio_interrupt_t edge, int timeout, int *val);


// if one thread needs to wait on more than 4 pins, increase this
#define  GPIO_MAX_MULTIPLE_WAIT_SOURCES 4

//
// Return  0 if triggered.
// Return -1 if timed out.
// Return -2 if static memory not allocated for more pins.
// Return -3 otherwise.
//
int gpio_wait_edge_multi(int *pins,                 // in:  a null terminated array of pins on which to wait.
                         gpio_interrupt_t *edges,   // in:  a null terminated array of edges matching pins.
                         int fd,                    // in:  an optional open file descriptor to wait on additonally.  -1 to ignore.
                         int timeout,               // in:  timeout in milliseconds
                         int *pin,                  // out:  which pin triggered.  Negative for fd.
                         int *val);                 // out:  value of pin after trigger


// when a thread is waiting on the GPIO line, use this to peek at the
// value without resetting the state.
// Will return error if pin is not waiting.
int gpio_peek(int num, int *val);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // __GPIOLIB_H_

