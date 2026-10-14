#include "spy.h"
#include <stdint.h>

typedef struct {
	bool tx_enable;
	uint32_t state;
} spy_t;

spy_t spy = {};

static const char *const cmd_names[] = { "start", "stop", "pause", NULL };


void spy_init(void) {
}

static int cmd_spy(cli_t *cli, const cli_args_t *args, void *user) {
	(void)cli; (void)args; (void)user;
    cli_write(cli, "Spy\n");
	return 0;
}

static int cmd_spy_start(cli_t *cli, const cli_args_t *args, void *user) {
	(void)cli; (void)args; (void)user;
    cli_write(cli, "Spy start\n");
	return 0;
}

static int cmd_spy_stop(cli_t *cli, const cli_args_t *args, void *user) {
	(void)cli; (void)args; (void)user;
    cli_write(cli, "Spy stop\n");
	return 0;
}

static int cmd_spy_pause(cli_t *cli, const cli_args_t *args, void *user) {
	(void)cli; (void)args; (void)user;
    cli_write(cli, "Spy pause\n");
	return 0;
}

static int cmd_spy_cmd(cli_t *cli, const cli_args_t *args, void *user) {
	(void)cli; (void)args; (void)user;
    spy.state = cli_arg_enum(args, 0, -1);
    switch (spy.state) {
	    case 0:
		    cli_write(cli, "Spy cmd start\n");
		    break;
	    case 1:
		    cli_write(cli, "Spy cmd stop\n");
		    break;
	    case 2:
		    cli_write(cli, "Spy cmd pause\n");
		    break;
    }
    cli_write(cli, "Spy cmd executed\n");
	return 0;
}

static int cmd_spy_tx_enable(cli_t *cli, const cli_args_t *args, void *user) {
	(void)cli; (void)user;
	spy.tx_enable = cli_arg_bool(args, 0, false);
    if (spy.tx_enable) {
    	cli_write(cli, "Spy Tx enabled\n");
    } else {
    	cli_write(cli, "Spy Tx disabled\n");
    }
	return 0;
}

static int cmd_spy_tx_fixed (cli_t *cli, const cli_args_t *args, void *user) {
	(void)cli; (void)user;
	uint32_t lenght = cli_arg_uint(args, 0, 1);
	uint32_t repeat = cli_arg_uint(args, 1, 1);
	while( repeat--) {
	    cli_printf(cli, "tx: %u bytes\n", lenght);
	}
	return 0;
}


static const cli_arg_spec_t cmd_args[] = {
    { "cmd", CLI_ARG_ENUM, true, false, cmd_names, "which cmd" },
};

static const cli_arg_spec_t spy_tx_enable_args[] = {
    { "enable", CLI_ARG_BOOL, false, false, NULL, "on/off, 1/0, yes/no" },
};

static const cli_arg_spec_t spy_tx_fixed_args[] = {
	{"size", CLI_ARG_UINT, false, false, NULL, "tgm size"},
	{"repeat", CLI_ARG_UINT, true, false, NULL, "tgm repeats"},
};

const cli_cmd_t spy_tx_subs[] = {
	{"enable", "enable", NULL, 0, spy_tx_enable_args, 1, cmd_spy_tx_enable},
	{"fix", "fixed size", NULL, 0, spy_tx_fixed_args, 1, cmd_spy_tx_fixed},
};

const cli_cmd_t spy_subs[] = {
	{"start", "start", NULL, 0, NULL, 0, cmd_spy_start},
	{"stop", "stop", NULL, 0, NULL, 0, cmd_spy_stop},
	{"pause", "pause", NULL, 0, NULL, 0, cmd_spy_pause},
	{"cmd", "hdlcspy", NULL, 0, cmd_args, 1, cmd_spy_cmd},
	{"send", "send", spy_tx_subs, sizeof(spy_tx_subs) / sizeof(spy_tx_subs[0]), NULL, 0, NULL},
};

const cli_cmd_t spy_cli_command = {
    "spy",  "HdlcSpy commands", spy_subs, sizeof(spy_subs) / sizeof(spy_subs[0]), NULL, 0, NULL
};
