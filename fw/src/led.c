#include "led.h"

#include <stdbool.h>

#include "hardware/gpio.h"
#include "pico/rand.h"
#include "pico/time.h"

const char *const led_names[] = { "led0", "led1", "led2", "all", NULL };

/* Blink periods, in milliseconds between toggles (half the visible period). */
#define LED_BLINK_SLOW_PERIOD_MS 500u
#define LED_BLINK_FAST_PERIOD_MS 100u

/* Router-style traffic flicker: short flash, irregular gap. */
#define LED_TRAFFIC_FLASH_MIN_MS 20u
#define LED_TRAFFIC_FLASH_MAX_MS 80u
#define LED_TRAFFIC_GAP_MIN_MS 60u
#define LED_TRAFFIC_GAP_MAX_MS 400u

typedef struct {
    uint32_t pin;
    led_mode_t mode;
    bool state;              /**< current GPIO level */
    uint32_t next_ms;         /**< next scheduled transition */
    bool traffic_flash_active; /**< traffic mode sub-state */
} led_t;

static led_t leds[LED_COUNT];

static void led_apply(led_t *led, bool on) {
    led->state = on;
    gpio_put(led->pin, on);
}

/** Uniformly distributed integer in [min, max], inclusive. */
static uint32_t rand_range(uint32_t min, uint32_t max) {
    uint32_t span = (max - min) + 1u;
    return min + (get_rand_32() % span);
}

void led_driver_init(void) {
    static const uint32_t pins[LED_COUNT] = { PICO_DEFAULT_LED_PIN, 17u, 18u };
    uint32_t i;

    for (i = 0u; i < LED_COUNT; i++) {
        leds[i].pin = pins[i];
        leds[i].mode = LED_MODE_OFF;
        leds[i].state = false;
        leds[i].next_ms = 0u;
        leds[i].traffic_flash_active = false;

        gpio_init(leds[i].pin);
        gpio_set_dir(leds[i].pin, GPIO_OUT);
        gpio_put(leds[i].pin, false);
    }
}

static void led_tick_one(led_t *led, uint32_t now_ms) {
    switch (led->mode) {
    case LED_MODE_OFF:
        if (led->state) {
            led_apply(led, false);
        }
        break;

    case LED_MODE_ON:
        if (!led->state) {
            led_apply(led, true);
        }
        break;

    case LED_MODE_BLINK_SLOW:
    case LED_MODE_BLINK_FAST: {
        uint32_t period = (led->mode == LED_MODE_BLINK_SLOW)
                               ? LED_BLINK_SLOW_PERIOD_MS
                               : LED_BLINK_FAST_PERIOD_MS;

        if ((int32_t)(now_ms - led->next_ms) >= 0) {
            led_apply(led, !led->state);
            led->next_ms = now_ms + period;
        }
        break;
    }

    case LED_MODE_TRAFFIC:
        if ((int32_t)(now_ms - led->next_ms) >= 0) {
            if (led->traffic_flash_active) {
                led_apply(led, false);
                led->next_ms = now_ms + rand_range(LED_TRAFFIC_GAP_MIN_MS, LED_TRAFFIC_GAP_MAX_MS);
                led->traffic_flash_active = false;
            } else {
                led_apply(led, true);
                led->next_ms = now_ms + rand_range(LED_TRAFFIC_FLASH_MIN_MS, LED_TRAFFIC_FLASH_MAX_MS);
                led->traffic_flash_active = true;
            }
        }
        break;

    default:
        break;
    }
}

void led_driver_tick(uint32_t now_ms) {
    uint32_t i;

    for (i = 0u; i < LED_COUNT; i++) {
        led_tick_one(&leds[i], now_ms);
    }
}

static void led_enter_mode(led_t *led, led_mode_t mode, uint32_t now_ms) {
    led->mode = mode;
    led->traffic_flash_active = false;
    led->next_ms = now_ms;

    /* Steady modes take effect immediately; blink/traffic apply on the next tick. */
    if (mode == LED_MODE_OFF) {
        led_apply(led, false);
    } else if (mode == LED_MODE_ON) {
        led_apply(led, true);
    }
}

void led_set_mode(uint32_t index, led_mode_t mode) {
    uint32_t now_ms;

    if (index >= LED_COUNT) {
        return;
    }
    now_ms = to_ms_since_boot(get_absolute_time());
    led_enter_mode(&leds[index], mode, now_ms);
}

void led_set_mode_all(led_mode_t mode) {
    uint32_t i;

    for (i = 0u; i < LED_COUNT; i++) {
        led_set_mode(i, mode);
    }
}

const char *led_mode_name(led_mode_t mode) {
    switch (mode) {
    case LED_MODE_OFF:
        return "off";
    case LED_MODE_ON:
        return "on";
    case LED_MODE_BLINK_SLOW:
        return "blink-slow";
    case LED_MODE_BLINK_FAST:
        return "blink-fast";
    case LED_MODE_TRAFFIC:
        return "traffic";
    default:
        return "unknown";
    }
}
