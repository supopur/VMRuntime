#pragma once
#include <sys/types.h>

///@brief Defines how many variables (uint32) can we hold in memory
#define VM_STACK_SIZE 256
///@brief Storage for local variables (uint32) in memory
#define VM_LOCALS_SIZE 64 //uint32 too

///@brief Bytecode instructions
///@attention Keep in sync with Compiler.h in VMCompiler
typedef enum {
    // Variables operations
    LOAD = 0x01,
    STORE = 0x02,
    // Stack manipulation
    POP = 0x03,
    DUP = 0x04,
    PUSH_CONST = 0x05,
    // Jumps
    JUMP = 0x06, JUMP_IF_FALSE = 0x07,
    // Functions
    RETURN = 0x08, CALL = 0x09,
    // Housekeeping
    HALT = 0xFF,

    // Math
    ADD = 0x10,
    SUB = 0x11,
    MUL = 0x12,
    DIV = 0x13,
    MOD = 0x14,

    // Comparison
    LT = 0x20,
    LE = 0x21,
    GT = 0x22,
    GE = 0x23,
    EQ = 0x24,
    NEQ = 0x25,

    // Logic
    AND = 0x30,
    OR = 0x31,
    NOT = 0x32,
    NEGATE = 0x33,
    // Bit operations
    BAND = 0x34,
    BOR = 0x35,
    BXOR = 0x36,
    BNOT = 0x37,
    SHL = 0x38,
    SHR = 0x39,
} vm_opcode_t;

///@brief Single instruction/operation
///@param byteCode Opcode
///@param operand Int argument, optional
///@param strOperand Char array/string argument, optional
///@attention Only one operand type can be defined
typedef struct {
    vm_opcode_t byteCode;
    u_int32_t operand;
    char strOperand[];
} vm_instruction_t;

///@brief Function metadata
typedef struct {
    uint32_t bytecode_offset;  // Offset into bytecode array
    uint32_t bytecode_size;    // Size in bytes
    uint32_t local_count;      // Number of local variables
    uint32_t param_count;      // Number of parameters
} vm_function_t;

///@brief Program bytecode and metadata
typedef struct {
    const uint8_t *bytecode;   // Read-only bytecode buffer
    size_t bytecode_size;      // Total bytecode size

    const vm_function_t *functions;  // Function table
    uint32_t function_count;

    const char * const *strings;  // String constant table
    uint32_t string_count;
} vm_program_t;

///@brief Virtual machine execution state/runtime
typedef struct {
    const vm_program_t *program;

    ///@brief Instruction Pointer
    ///@details Points to the current instruction's index inside the currently loaded program. Is zeroed out on creating/reset of a VM runtime
    size_t ip;              // instruction pointer

    uint32_t stack[VM_STACK_SIZE];
    size_t stack_top;

    uint32_t locals[VM_LOCALS_SIZE];

    bool halted;

} vm_t;