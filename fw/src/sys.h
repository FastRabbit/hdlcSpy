#pragma once

#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>

#include "cli.h"


void sys_init(void);

extern const cli_cmd_t sys_cli_comamnd;

void DEB(uint8_t pin, bool state);