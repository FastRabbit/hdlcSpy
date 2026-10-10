#include "tx.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include <cstddef>
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

const uint8_t hdlc_flag = 0x7E;
const uint8_t hdlc_abort = 0xFF;


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
    wait_clk_edge(high);
    gpio_put(tx_pin, bit ? 1u : 0u);
    wait_clk_edge(low);
}

void tx_send(uint8_t data, bool bit_stuff) {

    uint8_t bit_stuff_cnt = 0u;
    uint8_t bit_mask = 0x01u;

    for (uint8_t i = 0u; i < 8u; i++) {
        uint8_t bit = (data & bit_mask) ? 1u : 0u;
        if (bit_stuff) {
            if (bit) {
                bit_stuff_cnt++;
                if (bit_stuff_cnt >= 5u) {
                    tx_send_bit(0u);
                    bit_stuff_cnt = 0u;
                }
            } else {
                bit_stuff_cnt = 0u;
            }
        }
        tx_send_bit(bit);
        bit_mask <<= 1u;
    }
}


void cmd_tx_set(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli;
    (void)user;
    bool state = cli_arg_bool(args, 0u, false);
    gpio_put(tx_pin, state ? 1u : 0u);
}


void cmd_tx_abort(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli;
    (void)args;
    (void)user;
    tx_send(hdlc_abort, false);
}

void cli_cmd_tx_flag(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli;
    (void)args;
    (void)user;
    tx_send(hdlc_flag, false);
}

void cmd_tx_send(cli_t *cli, const cli_args_t *args, void *user) {
    (void)cli;
    (void)user;
    uint8_t data = (uint8_t)cli_arg_uint(args, 0u, 0u);
    tx_send(data, true);
}

static const cli_arg_spec_t tx_set_args[] = {
    { "state", CLI_ARG_BOOL, false, false, NULL, "on/off, true/false, yes/no, 1/0" },
};

static const cli_arg_spec_t tx_send_args[] = {
    { "state", CLI_ARG_UINT, false, false, NULL, "0..255" },
};

static const cli_cmd_t tx_subs[] = {
    { "set", "set the TX pin low/high", NULL, 0u, tx_set_args, 1u, cmd_tx_set },
    { "abort", "send the HDLC Abort ", NULL, 0u, NULL, 0u, cmd_tx_abort },
    { "flag", "send the HDLC Flag 0x7E", NULL, 0u, NULL, 0u, cmd_tx_flag },
    { "send", "send a byte over TX", NULL, 0u, tx_set_args, 1u, cmd_tx_send },
};    

const cli_cmd_t tx_clock_cli_command = {
    "tx", "TX bit bang commands", tx_subs,
    sizeof(tx_subs) / sizeof(tx_subs[0]), NULL, 0u, NULL
};