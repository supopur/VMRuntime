#include "../include/vm_runtime.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../../Core/Src/Orangutan/Drivers/usb_driver.h"

VM_t vm;
static VMProgram_t s_loadedProgram;

// Header metadata
static uint32_t variableCount = 0;
static uint32_t constantCount = 0;
static uint32_t constantOffset = 0;
static uint32_t functionCount = 0;
static uint32_t functionOffset = 0;
static uint32_t eventHandlerCount = 0;
static uint32_t eventHandlerOffset = 0;
static uint32_t instructionCount = 0;
static uint32_t bytecodeOffset = 0;

static inline bool vm_push(uint32_t value) {
    if (vm.stackTop >= VM_STACK_SIZE) {
        USB_PrintDebug("[VMR] ERR stack overflow\r\n");
        vm.halted = true;
        return false;
    }
    vm.stack[vm.stackTop++] = value;
    return true;
}

static inline bool vm_pop(uint32_t *p_value) {
    if (p_value == NULL || vm.stackTop == 0) {
        USB_PrintDebug("[VMR] ERR stack underflow\r\n");
        vm.halted = true;
        return false;
    }
    *p_value = vm.stack[--vm.stackTop];
    return true;
}

static uint32_t read_u32(size_t *p_offset) {
    uint32_t value = 0;
    if (vm.p_program != NULL && vm.p_program->p_bytecode != NULL) {
        memcpy(&value, (const uint8_t *)vm.p_program->p_bytecode + *p_offset, sizeof(value));
        *p_offset += sizeof(value);
    }
    return value;
}

static bool parse_header(void) {
    if (vm.p_program == NULL || vm.p_program->p_bytecode == NULL) {
        return false;
    }

    size_t offset = 0;
    char magic[5];
    memcpy(magic, (const uint8_t *)vm.p_program->p_bytecode + offset, 4);
    magic[4] = '\0';
    offset += 4;

    if (strcmp(magic, VMR_HEADER_MAGIC) != 0) {
        USB_PrintDebug("[VMR] ERR failed to find magic in bytecode header!\r\n");
        return false;
    }

    uint32_t version = read_u32(&offset);
    if (version > VMR_RUNTIME_VERSION) {
        USB_PrintDebug("[VMR] WARN compiler version greater than runtime version.\r\n");
    }

    // headerSize
    (void)read_u32(&offset);

    variableCount = read_u32(&offset);

    constantCount = read_u32(&offset);
    constantOffset = read_u32(&offset);

    vm.p_program->p_constants = (uint32_t *)((const uint8_t *)vm.p_program->p_bytecode + constantOffset);

    functionCount = read_u32(&offset);
    functionOffset = read_u32(&offset);

    vm.p_program->p_functions = (VMFunction_t *)((const uint8_t *)vm.p_program->p_bytecode + functionOffset);

    eventHandlerCount = read_u32(&offset);
    eventHandlerOffset = read_u32(&offset);

    if (eventHandlerCount == 0) {
        USB_PrintDebug("[VMR] ERR no event handlers registered in BC\r\n");
        return false;
    }

    vm.p_program->p_event_handlers = (VMEventHandler_t *)((const uint8_t *)vm.p_program->p_bytecode + eventHandlerOffset);

    instructionCount = read_u32(&offset);
    bytecodeOffset = read_u32(&offset);

    vm.p_program->p_instructions = (VMInstruction_t *)((const uint8_t *)vm.p_program->p_bytecode + bytecodeOffset);

    return true;
}

void VM_Destroy(void) {
    vm.ip = 0;
    vm.stackTop = 0;
    vm.halted = true;

    memset(vm.stack, 0, sizeof(vm.stack));
    memset(vm.locals, 0, sizeof(vm.locals));
}

void VM_LoadProgram(const void *p_bytecode) {
    VM_Destroy();

    memset(&s_loadedProgram, 0, sizeof(s_loadedProgram));
    s_loadedProgram.p_bytecode = p_bytecode;

    vm = (VM_t){
        .p_program = &s_loadedProgram,
        .halted = false,
        .stackTop = 0,
        .ip = 0,
    };

    if (parse_header()) {
        USB_PrintDebug("[VMR] INF BC header parsed\r\n");
    } else {
        USB_PrintDebug("[VMR] ERR BC header failed to parse\r\n");
        vm.halted = true;
    }
}

uint32_t VM_GetEventInstructionOffset(const VMEvent_t *p_event) {
    if (p_event == NULL || vm.p_program == NULL || vm.p_program->p_bytecode == NULL) {
        return 0;
    }

    for (uint32_t i = 0; i < eventHandlerCount; i++) {
        const VMEventHandler_t *p_handler = (const VMEventHandler_t *)((const uint8_t *)vm.p_program->p_bytecode + eventHandlerOffset + (i * sizeof(VMEventHandler_t)));

        if (p_handler->eventType == *p_event) {
            return p_handler->instructionOffset;
        }
    }

    return 0;
}

void VM_JumpToAddr(const size_t *p_target_addr) {
    if (p_target_addr != NULL) {
        vm.ip = *p_target_addr;
        vm.halted = false;
    }
}

bool VM_IsHalted(void) {
    return vm.halted;
}

bool VM_Step(void) {
    if (vm.halted || vm.p_program == NULL || vm.p_program->p_instructions == NULL) {
        return false;
    }

    if (vm.ip >= instructionCount) {
        vm.halted = true;
        return false;
    }

    VMInstruction_t instr = vm.p_program->p_instructions[vm.ip++];
    uint32_t a = 0;
    uint32_t b = 0;

    switch (instr.operation) {
        case PUSH_CONST:
            if (instr.operand < constantCount && vm.p_program->p_constants != NULL) {
                vm_push(vm.p_program->p_constants[instr.operand]);
            }
            break;

        case LOAD:
            if (instr.operand < VM_LOCALS_SIZE) {
                vm_push(vm.locals[instr.operand]);
            }
            break;

        case STORE:
            if (instr.operand < VM_LOCALS_SIZE && vm_pop(&a)) {
                vm.locals[instr.operand] = a;
            }
            break;

        case POP:
            vm_pop(&a);
            break;

        case DUP:
            if (vm.stackTop > 0) {
                vm_push(vm.stack[vm.stackTop - 1]);
            }
            break;

        case ADD:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a + b);
            break;

        case SUB:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a - b);
            break;

        case MUL:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a * b);
            break;

        case DIV:
            if (vm_pop(&b) && vm_pop(&a)) {
                if (b != 0) {
                    vm_push(a / b);
                } else {
                    USB_PrintDebug("[VMR] ERR division by zero\r\n");
                    vm.halted = true;
                }
            }
            break;

        case MOD:
            if (vm_pop(&b) && vm_pop(&a)) {
                if (b != 0) {
                    vm_push(a % b);
                } else {
                    USB_PrintDebug("[VMR] ERR modulo by zero\r\n");
                    vm.halted = true;
                }
            }
            break;

        case LT:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a < b) ? 1 : 0);
            break;

        case LE:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a <= b) ? 1 : 0);
            break;

        case GT:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a > b) ? 1 : 0);
            break;

        case GE:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a >= b) ? 1 : 0);
            break;

        case EQ:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a == b) ? 1 : 0);
            break;

        case NEQ:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a != b) ? 1 : 0);
            break;

        case AND:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a && b) ? 1 : 0);
            break;

        case OR:
            if (vm_pop(&b) && vm_pop(&a)) vm_push((a || b) ? 1 : 0);
            break;

        case NOT:
            if (vm_pop(&a)) vm_push((!a) ? 1 : 0);
            break;

        case NEGATE:
            if (vm_pop(&a)) vm_push((uint32_t)(-(int32_t)a));
            break;

        case BAND:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a & b);
            break;

        case BOR:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a | b);
            break;

        case BXOR:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a ^ b);
            break;

        case BNOT:
            if (vm_pop(&a)) vm_push(~a);
            break;

        case SHL:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a << b);
            break;

        case SHR:
            if (vm_pop(&b) && vm_pop(&a)) vm_push(a >> b);
            break;

        case JUMP:
            vm.ip = instr.operand;
            break;

        case JUMP_IF_FALSE:
            if (vm_pop(&a)) {
                if (a == 0) {
                    vm.ip = instr.operand;
                }
            }
            break;

        case HALT:
            vm.halted = true;
            return false;

        default:
            USB_PrintDebug("[VMR] ERR unknown opcode\r\n");
            vm.halted = true;
            return false;
    }

    return !vm.halted;
}

void tick(void) {
    VM_Step();
}