#include "../include/vm_runtime.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../../Core/Src/Orangutan/Drivers/usb_driver.h"

#define HEADER_MAGIC "VMBC"
#define RUNTIME_VERSION 1

VM_t vm;
//forward declaration
VMOpcode_t current();
bool parse_header();

//bytecode file header properties
// uint32_t headerSize;
uint32_t variableCount;
uint32_t constantCount;
uint32_t constantOffset;
uint32_t functionCount;
uint32_t functionOffset;
uint32_t eventHandlerCount;
uint32_t eventHandlerOffset;
uint32_t instructionCount;
uint32_t bytecodeOffset;
// uint32_t bytecodeSize;

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

void VM_LoadProgram(const void *p_program_file) {
    VM_Destroy();

    VMProgram_t program;

    vm = (VM_t){
        .p_program = &program,
        .halted = false,
        .stackTop = 0,
    };

    if (parse_header())
        USB_PrintDebug("[VMR] INF BC header parsed");
    else
        USB_PrintDebug("[VMR] ERR BC header failed to parse");
}

size_t VM_LookupEvent(VMEvent_t targetEvent) {
    uint8_t *base = (uint8_t *)vm.p_program->p_bytecode;
    VMEventHandler_t *handlers = (VMEventHandler_t *)(base + eventHandlerOffset);

    for (uint32_t i = 0; i < eventHandlerCount; i++) {
        VMEventHandler_t *current = &handlers[i];
        if (current->eventType == targetEvent)
            return i;
    }
}

void VM_JumpToAddr(const size_t *p_target_addr) {
    vm.ip = *p_target_addr;
    //todo clear jump table
}

///@brief Read 1 byte of instructions/operands
static uint8_t read_u8() {
    // read one byte and increment the instruction pointer before returning
    return ((uint8_t *)vm.p_program->p_bytecode)[vm.ip++];
}

///@brief Read char from current ip position and advance by 1 byte
static char read_char(void)
{
    return ((char *)vm.p_program->p_bytecode)[vm.ip++];
}

///@brief Read a string for len bytes
static void read_str(char *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len - 1; i++) {
        buf[i] = ((char *)vm.p_program->p_bytecode)[vm.ip++];
    }

    buf[len - 1] = '\0';
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

///@brief Gets current opcode and advances/consumes it
VMOpcode_t current() {
    return ((VMOpcode_t *)vm.p_program->p_bytecode)[vm.ip++];
}

void advance() {
    vm.ip++;
}

bool parse_header() {
    // we really shouldn't be using the IP for this but whatever
    vm.ip = 0;

    char magic[5];

    read_str(magic, 4);

    if (!strcmp(magic, HEADER_MAGIC)) {
        USB_PrintDebug("[VMR] ERR failed to find magic in bytecode header!");
        return false;
    }

    if (read_u32() > RUNTIME_VERSION)
        USB_PrintDebug("[VMR] WARN compiler version greater than runtime version.");

    // we don't really need this
    uint32_t headerSize = read_u32();

    variableCount = read_u32();

    constantCount = read_u32();
    constantOffset = read_u32();

    vm.p_program->p_constants = (uint32_t *)vm.p_program->p_bytecode + constantOffset;

    functionCount = read_u32();
    functionOffset = read_u32();

    vm.p_program->p_functions = (VMFunction_t *)vm.p_program->p_bytecode + functionOffset;

    eventHandlerCount = read_u32();
    eventHandlerOffset = read_u32();

    if (eventHandlerCount == 0) {
        USB_PrintDebug("[VMR] ERR no event handlers registered in BC");
        return false;
    }

    vm.p_program->p_event_handlers = (VMEventHandler_t *)(vm.p_program->p_bytecode + eventHandlerOffset);

    instructionCount = read_u32();
    bytecodeOffset = read_u32();

    vm.p_program->p_instructions = (VMInstruction_t *)(vm.p_program->p_bytecode + bytecodeOffset);

    // not needed for now
    // uint32_t bytecodeSize = read_u32();

    return true;
}