#pragma once

#include "vm_enums.h"

#define VMR_HEADER_MAGIC "VMBC"
#define VMR_RUNTIME_VERSION 1

void VM_Destroy();

void VM_LoadProgram(const void *p_bytecode);

///@brief Get the addr of a event in the currently loaded BC
///@details Returns 0 if event isn't registered, this equals to false
uint32_t VM_GetEventInstructionOffset(const VMEvent_t *p_event);

///@note This also clears the jump table
void VM_JumpToAddr(const size_t *p_target_addr);