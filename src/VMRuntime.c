#include "../include/VMRuntime.h"

#include <stdlib.h>
#include <string.h>

vm_t vm;

void vm_tickRuntime() {

}

void vm_destroy() {
    vm.ip = 0;
    vm.stack_top = 0;

    // Zero out the stack and locals storage
    memset(vm.stack, 0, VM_STACK_SIZE);
    memset(vm.locals, 0, VM_LOCALS_SIZE);
}

void vm_load_program(vm_program_t *program) {
    vm_destroy();

    vm = (vm_t){
        .program = program,
        .ip = 0,
        .halted = false,
        .stack_top = 0,

    };
}

vm_instruction_t current() {

}

void vm_advance() {
    vm.ip++;
}

static uint8_t vm_read_u8() {
    return vm_program[]
}

static uint16_t vm_read_u16() {

}

static uint32_t vm_read_u32() {

}