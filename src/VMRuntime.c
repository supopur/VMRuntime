#include "../include/VMRuntime.h"

#include <stdlib.h>
#include <string.h>

vm_t vm;
//forward declaration
vm_opcode_t current();

///@brief Processing method, should be called whenever possible
///@details If INC_FREERTOS_H is defined then this function will yield when possible, ideally this should be when it's finished with executing a single operation.
void vm_tickRuntime() {

    switch (current()) {
        case PUSH_CONST:
        {
            break;
        }

        default:
            break;
    }


    #ifdef INC_FREERTOS_H
        taskYIELD();
    #endif
}

void vm_destroy() {
    vm.ip = 0;
    vm.stack_top = 0;

    // Zero out the stack and locals storage
    memset(vm.stack, 0, VM_STACK_SIZE);
    memset(vm.locals, 0, VM_LOCALS_SIZE);
}

void vm_load_program(const vm_program_t *program) {
    vm_destroy();

    vm = (vm_t){
        .program = program,
        .halted = false,
        .stack_top = 0,
    };
}

///@brief Gets current opcode and advances/consumes it
vm_opcode_t current() {
    return (vm_opcode_t)vm.program->bytecode[vm.ip++];
}

void vm_advance() {
    vm.ip++;
}

///@brief Read 1 byte of instructions/operands
static uint8_t vm_read_u8() {
    // read one byte and increment the instruction pointer before returning
    return vm.program->bytecode[vm.ip++];
}

///@brief Read 2 bytes of instructions/operands
static uint16_t vm_read_u16() {
    uint16_t value;
    // copy the region of memory into the value variable
    memcpy(&value, vm.program->bytecode + vm.ip, sizeof(value));
    // move the instruction pointer to the end of what we have just read
    vm.ip += sizeof(value);
    return value;
}
///@brief Read 4 bytes of instructions/operands
static uint32_t vm_read_u32() {
    uint32_t value;
    // copy the region of memory into the value variable
    memcpy(&value, vm.program->bytecode + vm.ip, sizeof(value));
    // move the instruction pointer to the end of what we have just read
    vm.ip += sizeof(value);
    return value;
}