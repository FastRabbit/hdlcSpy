#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    uint32_t counter = 0;
    while (true) {
        printf("counter: %u\n", counter++);
        sleep_ms(1000);
    }

    return 0;
}
