#include "tx_clock.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

/** All of this module's mutable state, grouped so it reads as one unit. */
typedef struct {
    bool enabled;      /**< GPIO1 enable line driven high */
    bool output_on;    /**< PWM output on GPIO0 running */
    uint32_t hz;        /**< currently programmed frequency */
    uint pwm_slice;     /**< PWM slice backing TX_CLOCK_SIGNAL_PIN */
    uint pwm_channel;   /**< PWM channel (A/B) within that slice */
} tx_clock_state_t;

static tx_clock_state_t state = {
    .enabled = false,
    .output_on = false,
    .hz = TX_CLOCK_DEFAULT_HZ,
};

/**
 * @brief Reprograms the PWM slice's divider/wrap/level for a 50% duty square
 * wave at the given frequency.
 *
 * clk_sys / (divider * (wrap + 1)) = hz, with wrap limited to 16 bit and
 * divider limited to the PWM hardware's 8.4 fixed point range. Starts at
 * divider 1 and doubles it until wrap fits, which keeps the wrap count (and
 * thus timing resolution) as high as possible for the requested frequency.
 */
static void tx_clock_apply_frequency(uint32_t hz) {
    uint32_t sys_hz = clock_get_hz(clk_sys);
    float divider = 1.0f;
    uint32_t wrap;

    for (;;) {
        wrap = (uint32_t)((float)sys_hz / (divider * (float)hz)) - 1u;
        if (wrap <= 65535u || divider >= 255.0f) {
            break;
        }
        divider *= 2.0f;
    }
    if (divider > 255.0f) {
        divider = 255.0f;
    }
    if (wrap < 1u) {
        wrap = 1u;
    }

    pwm_set_clkdiv(state.pwm_slice, divider);
    pwm_set_wrap(state.pwm_slice, (uint16_t)wrap);
    pwm_set_chan_level(state.pwm_slice, state.pwm_channel, (uint16_t)((wrap + 1u) / 2u));
}

void tx_clock_driver_init(void) {
    gpio_init(TX_CLOCK_ENABLE_PIN);
    gpio_set_dir(TX_CLOCK_ENABLE_PIN, GPIO_OUT);
    gpio_put(TX_CLOCK_ENABLE_PIN, false);
    state.enabled = false;

    gpio_set_function(TX_CLOCK_SIGNAL_PIN, GPIO_FUNC_PWM);
    state.pwm_slice = pwm_gpio_to_slice_num(TX_CLOCK_SIGNAL_PIN);
    state.pwm_channel = pwm_gpio_to_channel(TX_CLOCK_SIGNAL_PIN);

    state.hz = TX_CLOCK_DEFAULT_HZ;
    tx_clock_apply_frequency(state.hz);
    pwm_set_enabled(state.pwm_slice, false);
    state.output_on = false;
}

void tx_clock_set_enabled(bool enabled) {
    state.enabled = enabled;
    gpio_put(TX_CLOCK_ENABLE_PIN, enabled);
}

bool tx_clock_is_enabled(void) {
    return state.enabled;
}

void tx_clock_set_output(bool on) {
    state.output_on = on;
    pwm_set_enabled(state.pwm_slice, on);
}

bool tx_clock_is_output_on(void) {
    return state.output_on;
}

uint32_t tx_clock_set_frequency(uint32_t hz) {
    if (hz < TX_CLOCK_MIN_HZ) {
        hz = TX_CLOCK_MIN_HZ;
    } else if (hz > TX_CLOCK_MAX_HZ) {
        hz = TX_CLOCK_MAX_HZ;
    }

    state.hz = hz;
    tx_clock_apply_frequency(hz);
    return hz;
}

uint32_t tx_clock_get_frequency(void) {
    return state.hz;
}

/* ------------------------------------------------------------------ */
/* clipilot `tx-clock` command                                          */
/* ------------------------------------------------------------------ */

static int cmd_tx_clock_enable(cli_t *cli, const cli_args_t *args, void *user) {
    (void)args;
    (void)user;
    tx_clock_set_enabled(true);
    cli_printf(cli, "tx-clock enable -> on (GPIO%u)\n", (unsigned)TX_CLOCK_ENABLE_PIN);
    return 0;
}

static int cmd_tx_clock_disable(cli_t *cli, const cli_args_t *args, void *user) {
    (void)args;
    (void)user;
    tx_clock_set_enabled(false);
    cli_printf(cli, "tx-clock enable -> off (GPIO%u)\n", (unsigned)TX_CLOCK_ENABLE_PIN);
    return 0;
}

static int cmd_tx_clock_on(cli_t *cli, const cli_args_t *args, void *user) {
    (void)args;
    (void)user;
    tx_clock_set_output(true);
    cli_printf(cli, "tx-clock on -> %u Hz on GPIO%u\n", (unsigned)tx_clock_get_frequency(),
               (unsigned)TX_CLOCK_SIGNAL_PIN);
    return 0;
}

static int cmd_tx_clock_off(cli_t *cli, const cli_args_t *args, void *user) {
    (void)args;
    (void)user;
    tx_clock_set_output(false);
    cli_printf(cli, "tx-clock off (GPIO%u)\n", (unsigned)TX_CLOCK_SIGNAL_PIN);
    return 0;
}

static const cli_arg_spec_t tx_clock_frequency_args[] = {
    { "hz", CLI_ARG_UINT, false, false, NULL, "1000..1000000, clamped if out of range" },
};

static int cmd_tx_clock_frequency(cli_t *cli, const cli_args_t *args, void *user) {
    uint32_t requested;
    uint32_t applied;

    (void)user;
    requested = cli_arg_uint(args, 0u, TX_CLOCK_DEFAULT_HZ);
    applied = tx_clock_set_frequency(requested);

    if (applied != requested) {
        cli_printf(cli, "requested %u Hz out of range, clamped to %u Hz\n", (unsigned)requested,
                   (unsigned)applied);
    } else {
        cli_printf(cli, "tx-clock frequency -> %u Hz\n", (unsigned)applied);
    }
    return 0;
}

static int cmd_tx_clock_status(cli_t *cli, const cli_args_t *args, void *user) {
    (void)args;
    (void)user;
    cli_printf(cli, "tx-clock: enable=%s output=%s frequency=%u Hz\n",
               tx_clock_is_enabled() ? "on" : "off", tx_clock_is_output_on() ? "on" : "off",
               (unsigned)tx_clock_get_frequency());
    return 0;
}

static const cli_cmd_t tx_clock_subs[] = {
    { "enable", "drive the TX clock enable line (GPIO1)", NULL, 0u, NULL, 0u, cmd_tx_clock_enable },
    { "disable", "release the TX clock enable line (GPIO1)", NULL, 0u, NULL, 0u, cmd_tx_clock_disable },
    { "on", "start the TX clock signal (GPIO0)", NULL, 0u, NULL, 0u, cmd_tx_clock_on },
    { "off", "stop the TX clock signal (GPIO0)", NULL, 0u, NULL, 0u, cmd_tx_clock_off },
    { "frequency", "set the TX clock frequency in Hz", NULL, 0u, tx_clock_frequency_args, 1u,
      cmd_tx_clock_frequency },
    { "status", "show enable/output/frequency state", NULL, 0u, NULL, 0u, cmd_tx_clock_status },
};

const cli_cmd_t tx_clock_cli_command = {
    "tx-clock", "TX clock control (enable=GPIO1, signal=GPIO0, 1kHz..1MHz)", tx_clock_subs,
    sizeof(tx_clock_subs) / sizeof(tx_clock_subs[0]), NULL, 0u, NULL
};
