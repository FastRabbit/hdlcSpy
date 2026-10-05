#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"

#include "cli.h"

/* ------------------------------------------------------------------ */
/* Console I/O over USB CDC                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Non-blocking character source.
 *
 * A zero timeout makes this return a negative value (no byte pending), which
 * is exactly what cli_poll() expects.
 */
static int cdc_getc(void *ctx) {
    (void)ctx;
    return getchar_timeout_us(0);
}

/**
 * @brief Character sink.
 *
 * putchar_raw() bypasses the SDK's own newline translation; cli_config_t::crlf
 * already expands '\n' to "\r\n", so plain putchar() would double it up.
 */
static void cdc_putc(void *ctx, char c) {
    (void)ctx;
    (void)putchar_raw(c);
}

/** Writes a banner before the CLI exists, so it lands above the first prompt. */
static void cdc_banner(const char *s) {
    while (*s != '\0') {
        if (*s == '\n') {
            (void)putchar_raw('\r');
        }
        (void)putchar_raw(*s);
        s++;
    }
}

static void print_banner(void) {
    cdc_banner("\n");
    cdc_banner(" _          _ _      ____              \n");
    cdc_banner("| |__    __| | | ___/ ___| _ __  _   _  \n");
    cdc_banner("| '_ \\  / _` | |/ __\\___ \\| '_ \\| | | | \n");
    cdc_banner("| | | || (_| | | (__ ___) | |_) | |_| | \n");
    cdc_banner("|_| |_| \\__,_|_|\\___|____/| .__/ \\__, | \n");
    cdc_banner("                          |_|    |___/  \n");
    cdc_banner(" USB CDC connected -- hdlcspy_fw ready\n");
    cdc_banner("\n");
}

/* ------------------------------------------------------------------ */
/* Console                                                              */
/* ------------------------------------------------------------------ */

static cli_t cli;

/**
 * @brief Sets up the CLI instance with the built-in commands only (help,
 * history). No application commands are registered yet.
 */
static void console_init(void) {
    cli_config_t cfg = cli_config_default();

    cfg.commands = NULL;
    cfg.command_count = 0u;
    cfg.getc_fn = cdc_getc;
    cfg.putc_fn = cdc_putc;
    cfg.prompt = "hdlcspy> ";
    cfg.echo = true;
    cfg.crlf = true;

    cli_init(&cli, &cfg);
}

int main(void) {
    bool usb_was_connected = false;

    stdio_init_all();

    /*
     * Anything written before the host opens the CDC port is discarded, so
     * the console is (re-)initialised on each connect. That also gives a
     * terminal attached long after boot a banner and a prompt.
     */
    for (;;) {
        bool connected = stdio_usb_connected();

        if (connected && !usb_was_connected) {
            print_banner();
            console_init();
        }
        usb_was_connected = connected;

        if (connected) {
            cli_poll(&cli);
        }
    }

    return 0;
}
