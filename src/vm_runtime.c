#include "../include/vm_runtime.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

VM_t vm;
//forward declaration
VMOpcode_t current();

///@brief Processing method, should be called whenever possible
void tick() {

    switch (current()) {
        case PUSH_CONST:
        {
            break;
        }

        default:
            break;
    }
}

void VM_Destroy() {
    vm.ip = 0;
    vm.stackTop = 0;

    // Zero out the stack and locals storage
    memset(vm.stack, 0, VM_STACK_SIZE);
    memset(vm.locals, 0, VM_LOCALS_SIZE);
}

void VM_LoadProgram(const VMProgram_t *p_program) {
    VM_Destroy();

    vm = (VM_t){
        .p_program = p_program,
        .halted = false,
        .stackTop = 0,
    };
}

///@brief Gets current opcode and advances/consumes it
VMOpcode_t current() {
    return (VMOpcode_t)vm.p_program->p_bytecode[vm.ip++];
}

void advance() {
    vm.ip++;
}

///@brief Read 1 byte of instructions/operands
static uint8_t read_u8() {
    // read one byte and increment the instruction pointer before returning
    return vm.p_program->p_bytecode[vm.ip++];
}

///@brief Read 2 bytes of instructions/operands
static uint16_t read_u16() {
    uint16_t value;
    // copy the region of memory into the value variable
    memcpy(&value, vm.p_program->p_bytecode + vm.ip, sizeof(value));
    // move the instruction pointer to the end of what we have just read
    vm.ip += sizeof(value);
    return value;
}
///@brief Read 4 bytes of instructions/operands
static uint32_t read_u32() {
    uint32_t value;
    // copy the region of memory into the value variable
    memcpy(&value, vm.p_program->p_bytecode + vm.ip, sizeof(value));
    // move the instruction pointer to the end of what we have just read
    vm.ip += sizeof(value);
    return value;
}