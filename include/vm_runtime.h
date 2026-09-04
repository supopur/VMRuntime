#pragma once

#include "vm_enums.h"

void VM_Destroy();

void VM_LoadProgram(const void *p_program_file);

///@brief Get the addr of a event in the currently loaded BC
///@details Returns 0 if event isn't registered, this equals to false
size_t VM_LookupEvent(VMEvent_t targetEvent);

///@note This also clears the jump table
void VM_JumpToAddr(const size_t *p_target_addr);