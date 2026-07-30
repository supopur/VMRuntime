#pragma once
#include <sys/types.h>


///@brief Bytecode instructions
///@attention Keep in sync with Compiler.h in VMCompiler
enum ByteCode {
    // Math
    ADD, SUB, MUL, DIV, MOD,
    // Comparison
    LT, LE, GT, GE, EQ, NEQ,
    // Variables operations
    LOAD, STORE,
    // Stack manipulation
    POP, DUP, PUSH_CONST,
    // Logic
    AND, OR, NOT, NEGATE,
    // Bit operations
    BAND, BOR, BXOR, BNOT, SHL, SHR,
    // Jumps
    JUMP, JUMP_IF_FALSE,
    // Functions
    RETURN, CALL,
    // Housekeeping
    HALT
};

struct Instruction {
    enum ByteCode byteCode;
    u_int32_t operand;
    char strOperand[];
};