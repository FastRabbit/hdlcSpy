#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cli.h"

/** GPIO driving the TX clock enable signal (gates an external clock buffer). */
#define TX_CLOCK_ENABLE_PIN 1u

/** GPIO carrying the TX clock signal itself (PWM square wave). */
#define TX_CLOCK_SIGNAL_PIN 0u

/** Allowed TX clock frequency range, inclusive. */
#define TX_CLOCK_MIN_HZ 1000u
#define TX_CLOCK_MAX_HZ 1000000u

/** Frequency programmed at boot, before any `tx-clock frequency` command. */
#define TX_CLOCK_DEFAULT_HZ 10000u

/**
 * @brief Configures the enable GPIO and the PWM slice backing the clock
 * signal. Both the enable line and the clock output start disabled, with
 * the frequency preset to TX_CLOCK_DEFAULT_HZ.
 */
void tx_clock_driver_init(void);

/** @brief Drives (true) or releases (false) the TX clock enable line (GPIO1). */
void tx_clock_set_enabled(bool enabled);

/** @brief Returns whether the TX clock enable line is currently driven. */
bool tx_clock_is_enabled(void);

/** @brief Starts (true) or stops (false) the PWM clock signal on GPIO0. */
void tx_clock_set_output(bool on);

/** @brief Returns whether the PWM clock signal is currently running. */
bool tx_clock_is_output_on(void);

/**
 * @brief Sets the TX clock frequency, clamped to
 * [TX_CLOCK_MIN_HZ, TX_CLOCK_MAX_HZ].
 * @return the frequency actually applied, after clamping.
 */
uint32_t tx_clock_set_frequency(uint32_t hz);

/** @brief Returns the currently programmed TX clock frequency, in Hz. */
uint32_t tx_clock_get_frequency(void);

/**
 * @brief The `tx-clock` clipilot command (enable/disable, on/off, frequency,
 * and status subcommands already wired up).
 *
 * Drop this into an application's cli_cmd_t registry alongside other
 * modules' singletons, e.g. in main.c's app_command_sources[]:
 * @code
 * static const cli_cmd_t *const app_command_sources[] = {
 *     &led_cli_command,
 *     &tx_clock_cli_command,
 * };
 * @endcode
 */
extern const cli_cmd_t tx_clock_cli_command;
