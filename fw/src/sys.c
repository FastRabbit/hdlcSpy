#include "sys.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"


void sys_init(void) {

}

static int cmd_sys_info(cli_t *cli, const cli_args_t *args, void *user) {
    (void)args; (void)user;
    cli_write(cli,  "board   : Raspberry Pi Pico 2 (RP2350, Cortex-M33)\n");
    cli_write(cli,  "console : USB CDC\n");
    cli_printf(cli, "uptime  : %lu ms\n", (unsigned long)to_ms_since_boot(get_absolute_time()));
    cli_printf(cli, "cli_t   : %u bytes\n", (unsigned)sizeof(cli_t));
    return 0;
}

static int cmd_sys_reboot(cli_t *cli, const cli_args_t *args, void *user) {
    bool bootsel = (cli_arg_enum(args, 0u, -1) == 0);

    (void)user;
    if (bootsel) {
        cli_write(cli, "rebooting into BOOTSEL\n");
        sleep_ms(50);
        reset_usb_boot(0u, 0u);
    }
    cli_write(cli, "rebooting\n");
    sleep_ms(50);
    watchdog_reboot(0u, 0u, 0u);
    return 0;
}

static const char *const reboot_targets[] = { "bootsel", NULL };

static const cli_arg_spec_t reboot_args[] = {
    { "target", CLI_ARG_ENUM, true, false, reboot_targets,
      "reboot into the USB bootloader" },
};

static const cli_cmd_t sys_subs[] = {
    { "info",   "board and console information", NULL, 0, NULL, 0, cmd_sys_info },
    { "reboot", "restart the MCU", NULL, 0, reboot_args, 1, cmd_sys_reboot },
};

const cli_cmd_t sys_cli_comamnd = {
    "sys", "system commands", sys_subs, sizeof(sys_subs) / sizeof(sys_subs[0]), NULL, 0, NULL
};
