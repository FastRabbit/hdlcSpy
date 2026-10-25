#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cli.h"

void spy_init(void);

extern const cli_cmd_t spy_cli_command;
