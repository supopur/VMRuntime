#pragma once

#include "Enums.h"

void vm_tickRuntime();
void vm_destroy();
void vm_load_program(const vm_program_t *program);
