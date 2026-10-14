#include "tx.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "sys.h"
#include "tx_clock.h"
#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>

/*
    GPIO0 Clk Tx        Out
    GPIO1 Enable Tx     Out
    GPIO2 Clk Rx        In
    GPIO3 Data Tx       Out
    GPIO4 Data Rx       In
*/

/*
    TX 0    Driver Disable   Bus Low
    TX 1    Driver Enable   Bus High
 */

const uint8_t rx_clock_pin = 2u;
const uint8_t tx_pin = 3u;
const uint8_t rx_pin = 4u;

const uint16_t hdlc_flag = 0x007E;
const uint16_t hdlc_abort = 0x00FF;
const uint16_t hdlc_idle = 0x03FF;

typedef struct {
    bool tx_inverted;
} tx_state_t;

static tx_state_t tx_state = {
    .tx_inverted = false,
};

void tx_driver_init(void) {
    gpio_init(tx_pin);
    gpio_set_dir(tx_pin, GPIO_OUT);
    gpio_put(tx_pin, 0u);
    gpio_init(rx_clock_pin);
    gpio_set_dir(rx_clock_pin, GPIO_IN);
}

void wait_clk_edge(bool high) {
    while (gpio_get(rx_clock_pin) != (high ? 1u : 0u)) {
        tight_loop_contents();
    }
}

void tx_send_bit(uint8_t bit) {
    wait_clk_edge(true);
    if (tx_state.tx_inverted) {
        gpio_put(tx_pin, bit ? 0u : 1u);
    } else {
        gpio_put(tx_pin, bit ? 1u : 0u);
    }
    wait_clk_edge(false);
}

void tx_send_byte(uint8_t byte, bool bit_stuff) {
    uint8_t bit_stuff_cnt = 0u;
    uint8_t bit_mask = 0x01u;
    DEB(8, true);
    for (uint_fast8_t i = 0u; i < 8u; i++) {
        uint8_t bit = (byte & bit_mask) ? 1u : 0u;
        if (bit_stuff) {
            if (bit) {
                bit_stuff_cnt++;
                if (bit_stuff_cnt > 5u) {
                    DEB(9, true);
                    tx_send_bit(0u);
                    bit_stuff_cnt = 0u;
                    DEB(9, false);
                }
            } else {
                bit_stuff_cnt = 0u;
            }
        }
        tx_send_bit(bit);
        bit_mask <<= 1u;
    }
    DEB(8, false);
}

void tx_cmd(uint16_t data, uint8_t length) {
    uint16_t bit_mask = 0x0001u;
    for (uint_fast8_t i = 0u; i < length; i++) {
        uint8_t bit = (data & bit_mask) ? 1u : 0u;
        tx_send_bit(bit);
        bit_mask <<= 1u;
    }
}

void tx_send(uint8_t data, uint32_t count, bool bit_stuff) {

    for (uint32_t loop = 0; loop < count; loop++) {
        tx_send_byte(data, bit_stuff);
    }
}

void send_frame(uint16_t length, uint8_t data) {
    tx_cmd(hdlc_flag, 8);
    for(uint_fast16_t i=0; i<length; i++) {
       tx_send(data, 1, true);
    }
    tx_cmd(hdlc_flag, 8);
}

static int cmd_tx_abort(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli; (void)args; (void)user;
    tx_cmd(hdlc_abort, 8);
    return 0;
}

static int cmd_tx_flag(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli; (void)args; (void)user;
    tx_cmd(hdlc_flag, 8);
    return 0;
}

static int cmd_tx_send(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli; (void)user;
    uint8_t data = (uint8_t) cli_arg_uint(args, 0, 0);
    uint32_t count = (uint32_t) cli_arg_uint(args, 1, 1);
    tx_send(data, count, true);
    return 0;
}

static int cmd_tx(cli_t *cli, const cli_args_t *args, void *user) {
    (void)args; (void)user;
    cli_printf(cli, "tx: inverte=%s \n", tx_state.tx_inverted ? "on" : "off");
    return 0;
}

static int cmd_tx_invert(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli; (void)user;
    tx_state.tx_inverted = cli_arg_bool(args, 0u, false);
    return 0;
}

static int cmd_tx_frame(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli; (void)user;
    uint16_t length = (uint16_t) cli_arg_uint(args, 0, 0);
    uint8_t data = (uint8_t) cli_arg_uint(args, 1, 0);
    uint32_t count = cli_arg_uint(args, 2, 1);
    while(count--) {
        send_frame(length, data);
        tx_cmd(hdlc_idle, 10);
    }
    return 0;
}

static const cli_arg_spec_t tx_set_args[] = {
    { "state", CLI_ARG_BOOL, false, false, NULL, "on/off, true/false, yes/no, 1/0" },
};

static const cli_arg_spec_t tx_send_args[] = {
    { "data", CLI_ARG_UINT, false, false, NULL, "data byte to send 0..255" },
    { "count", CLI_ARG_UINT, true, false, NULL, "number of repeats, default to 1"},
};

static const cli_arg_spec_t tx_frame_args[] = {
    { "length", CLI_ARG_UINT, false, false, NULL, "frame lenght"},
    { "data", CLI_ARG_UINT, false, false, NULL, "data to send 0..255" },
    { "count", CLI_ARG_UINT, true, false, NULL, "number of repeats, default to 1"},
};

static const cli_cmd_t tx_subs[] = {
    { "abort", "send the HDLC Abort ", NULL, 0u, NULL, 0u, cmd_tx_abort },
    { "flag", "send the HDLC Flag 0x7E", NULL, 0u, NULL, 0u, cmd_tx_flag },
    { "send", "send a byte over TX", NULL, 0u, tx_send_args, 2u, cmd_tx_send },
    { "invert", "invert TX",  NULL, 0u, tx_set_args, 1u, cmd_tx_invert },
    { "frame", "send frame",  NULL, 0u, tx_frame_args, 3u, cmd_tx_frame },
};

const cli_cmd_t tx_cli_command = {
    "tx", "TX bit bang commands", tx_subs,
    sizeof(tx_subs) / sizeof(tx_subs[0]), NULL, 0u, cmd_tx
};
