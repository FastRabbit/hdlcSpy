#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"

#include "cli.h"
#include "led.h"

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
/* LED commands                                                        */
/* ------------------------------------------------------------------ */

static const cli_arg_spec_t led_which_args[] = {
    { "which", CLI_ARG_ENUM, true, false, led_names,
      "led0 (onboard), led1 (GPIO17), led2 (GPIO18), or all (default all)" },
};

/** Shared implementation behind every led subcommand. */
static int led_apply_cmd(cli_t *cli, const cli_args_t *args, led_mode_t mode) {
    int which = cli_arg_enum(args, 0u, LED_ARG_ALL);

    if (which == LED_ARG_ALL) {
        led_set_mode_all(mode);
    } else {
        led_set_mode((uint32_t)which, mode);
    }

    cli_printf(cli, "%s -> %s\n", led_names[which], led_mode_name(mode));
    return 0;
}

static int cmd_led_on(cli_t *cli, const cli_args_t *args, void *user) {
    (void)user;
    return led_apply_cmd(cli, args, LED_MODE_ON);
}

static int cmd_led_off(cli_t *cli, const cli_args_t *args, void *user) {
    (void)user;
    return led_apply_cmd(cli, args, LED_MODE_OFF);
}

static int cmd_led_blink_slow(cli_t *cli, const cli_args_t *args, void *user) {
    (void)user;
    return led_apply_cmd(cli, args, LED_MODE_BLINK_SLOW);
}

static int cmd_led_blink_fast(cli_t *cli, const cli_args_t *args, void *user) {
    (void)user;
    return led_apply_cmd(cli, args, LED_MODE_BLINK_FAST);
}

static int cmd_led_traffic(cli_t *cli, const cli_args_t *args, void *user) {
    (void)user;
    return led_apply_cmd(cli, args, LED_MODE_TRAFFIC);
}

static const cli_cmd_t led_subs[] = {
    { "on", "steady on", NULL, 0u, led_which_args, 1u, cmd_led_on },
    { "off", "steady off", NULL, 0u, led_which_args, 1u, cmd_led_off },
    { "blink-slow", "blink at ~1 Hz", NULL, 0u, led_which_args, 1u, cmd_led_blink_slow },
    { "blink-fast", "blink at ~5 Hz", NULL, 0u, led_which_args, 1u, cmd_led_blink_fast },
    { "traffic", "irregular router-style activity flicker", NULL, 0u, led_which_args, 1u, cmd_led_traffic },
};

static const cli_cmd_t commands[] = {
    { "led", "LED control (led0=onboard, led1=GPIO17, led2=GPIO18)", led_subs,
      sizeof(led_subs) / sizeof(led_subs[0]), NULL, 0u, NULL },
};

/* ------------------------------------------------------------------ */
/* Console                                                              */
/* ------------------------------------------------------------------ */

static cli_t cli;

/**
 * @brief Sets up the CLI instance with the built-in commands (help, history)
 * plus the `led` command for LED control.
 */
static void console_init(void) {
    cli_config_t cfg = cli_config_default();

    cfg.commands = commands;
    cfg.command_count = sizeof(commands) / sizeof(commands[0]);
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
