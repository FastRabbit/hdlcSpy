#pragma once

#include <stdint.h>

#include "cli.h"

/** Number of LEDs the driver controls: led0 (onboard), led1, led2. */
#define LED_COUNT 3u

/** Per-LED behavior. */
typedef enum {
    LED_MODE_OFF = 0,     /**< steady off */
    LED_MODE_ON,           /**< steady on */
    LED_MODE_BLINK_SLOW,   /**< ~1 Hz square wave */
    LED_MODE_BLINK_FAST,   /**< ~5 Hz square wave */
    LED_MODE_TRAFFIC,      /**< irregular router-style activity flicker */
} led_mode_t;

/** NULL terminated names for CLI enum arguments: "led0", "led1", "led2", "all". */
extern const char *const led_names[];

/** Index of "all" within led_names, for CLI enum fallbacks. */
#define LED_ARG_ALL 3

/**
 * @brief Initializes the GPIO pins for all LEDs and sets them off.
 *
 * led0 is the onboard LED (PICO_DEFAULT_LED_PIN); led1 is GPIO17; led2 is
 * GPIO18.
 */
void led_driver_init(void);

/**
 * @brief Advances the software blink/traffic state machines.
 *
 * Must be called regularly (e.g. every main loop iteration) regardless of
 * whether a console is attached, so LEDs keep behaving even without a host
 * connected.
 */
void led_driver_tick(uint32_t now_ms);

/** @brief Sets the mode of one LED (0 .. LED_COUNT-1). Out of range is a no-op. */
void led_set_mode(uint32_t index, led_mode_t mode);

/** @brief Sets the mode of all LEDs at once. */
void led_set_mode_all(led_mode_t mode);

/** @brief Human readable name for a mode, e.g. for CLI output. */
const char *led_mode_name(led_mode_t mode);

/**
 * @brief The `led` clipilot command (with its on/off/blink-slow/blink-fast/
 * traffic subcommands already wired up).
 *
 * Drop this straight into an application's cli_cmd_t table, e.g.:
 * @code
 * static const cli_cmd_t commands[] = { led_cli_command };
 * @endcode
 */
extern const cli_cmd_t led_cli_command;

