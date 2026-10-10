#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"

#include "cli.h"
#include "led.h"
#include "tx_clock.h"
#include "tx.h"

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

/**
 * @brief Top-level command registry.
 *
 * clipilot requires cli_config_t::commands to point at one contiguous
 * cli_cmd_t array, so every module's singleton `xxx_cli_command` object is
 * copied into app_commands[] here. To add a module, give it its own
 * `extern const cli_cmd_t xxx_cli_command;` (see led.c/led.h) and add one
 * line below -- no other file needs to change.
 */
static const cli_cmd_t *const app_command_sources[] = {
    &led_cli_command,
    &tx_clock_cli_command,
    &tx_cli_command,
};
#define APP_COMMAND_COUNT (sizeof(app_command_sources) / sizeof(app_command_sources[0]))

static cli_cmd_t app_commands[APP_COMMAND_COUNT];

static void app_commands_init(void) {
    size_t i;

    for (i = 0u; i < APP_COMMAND_COUNT; i++) {
        app_commands[i] = *app_command_sources[i];
    }
}

static cli_t cli;

/**
 * @brief Sets up the CLI instance with the built-in commands (help, history)
 * plus the application commands assembled into app_commands[].
 */
static void console_init(void) {
    cli_config_t cfg = cli_config_default();

    cfg.commands = app_commands;
    cfg.command_count = APP_COMMAND_COUNT;
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
    led_driver_init();
    tx_clock_driver_init();
    tx_driver_init();
    app_commands_init();

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

        /* Keeps blinking/traffic patterns alive even without a host attached. */
        led_driver_tick(to_ms_since_boot(get_absolute_time()));
    }

    return 0;
}
