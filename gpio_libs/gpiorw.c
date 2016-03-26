#include <stdio.h>
#include <stdlib.h>

#include "gpiolib.h"

static void usage (FILE * fp, int argc, char **argv)
{
    fprintf (fp,
       "Usage: %s pin [value]\n\n", argv[0]);
}

int main(int argc, char **argv)
{
    int pin = -1;
    int value = -1;

    if (argc < 2)
    {
        usage(stdout, argc, argv);
        return 0;
    }

    pin = strtol(argv[1], NULL, 0);

    gpio_init();

    if (argc == 2)
    {
        printf("Get gpio pin %d value: %d \n", pin, gpio_get_value(pin));
    }
    else
    {
        value = strtol(argv[2], NULL, 0);
        gpio_set_value(pin, value);
        printf("Set gpio pin %d value: %d \n", pin, value);
    }

    gpio_term();

    return 0;
}
