#include <stdio.h>
#include "pico/stdlib.h"

static void print_banner(void) {
    printf("\n");
    printf(" _          _ _      ____              \n");
    printf("| |__    __| | | ___/ ___| _ __  _   _  \n");
    printf("| '_ \\  / _` | |/ __\\___ \\| '_ \\| | | | \n");
    printf("| | | || (_| | | (__ ___) | |_) | |_| | \n");
    printf("|_| |_| \\__,_|_|\\___|____/| .__/ \\__, | \n");
    printf("                          |_|    |___/  \n");
    printf(" USB CDC connected -- hdlcspy_fw ready\n");
    printf("\n");
}

int main(void) {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    print_banner();

    uint32_t counter = 0;
    while (true) {
        printf("counter: %u\n", counter++);
        sleep_ms(1000);
    }

    return 0;
}
