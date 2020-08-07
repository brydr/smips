/* ........... COMP1521 20T2 --- assignment 2: SMIPS, Simple MIPS .......... */
//               Written by Benjamin Ryder (z5207266), July 2020.
//             See 'main.c' for description of action of program.

/* ------------------- Instructions Implementation File -------------------- */
// * Implements functions which emulate the action of a MIPS instruction on a
//   set of registers, the program counter, and stdin/stdout. 

#include "instructions.h"

// Emulates signed integer addition of two registers.
// $d = $s + $t;    PC++
int32_t add(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] + reg[curr->reg_T];
    return 1;
}

// Emulates signed integer subtraction of two registers.
// $d = $s - $t;    PC++
int32_t sub(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] - reg[curr->reg_T];
    return 1;
}

// Emulates bitwise AND of two registers.
// $d = $s & $t;    PC++
int32_t and(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] & reg[curr->reg_T];
    return 1;
}

// Emulates bitwise OR of two registers.
// $d = $s | $t;    PC++
int32_t or(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] | reg[curr->reg_T];
    return 1;
}

// Emulates less-than expression of two registers.
// $d = $s < $t;    PC++
int32_t slt(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = (reg[curr->reg_S] < reg[curr->reg_T]);
    return 1;
}

// Emulates multiplication of two registers.
// $d = $s * $t;    PC++
int32_t mul(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] * reg[curr->reg_T];
    return 1;
}

// Emulates branching instruction for '==' comparison of two registers.
// if ($s == $t);   PC += I
int32_t beq(int32_t *reg, instr_t *curr) {
    /* if (reg[a1] == reg[a2]) {return (int16_t)a3} else {return 1} */
    uint8_t is_eq  = reg[curr->reg_S] == reg[curr->reg_T];
    return  1 + (is_eq)*((int16_t)curr->arg_3 - 1 );
}

// Emulates branching instruction for '!=' comparison of two registers.
// if ($s != $t);   PC += I
int32_t bne(int32_t *reg, instr_t *curr) {
    /* if (reg[a1] != reg[a2]) {return (int16_t)a3} else {return 1} */
    uint8_t is_neq = reg[curr->reg_S] != reg[curr->reg_T];
    return  1 + (is_neq)*((int16_t)curr->arg_3 - 1 );
}

// Emulates signed integer addition of a register and a 16b signed int literal.
// $t = $s + I;    PC++
int32_t addi(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] + (int16_t)curr->arg_3;
    return 1;
}

// Signal if a register is less than a 16-bit signed int literal.
// $t = $s < I;    PC++
int32_t slti(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] << curr->arg_3;
    return 1;
}

// Emulates bitwise AND between a register and a 16-bit signed int literal.
// $t = $s & I;    PC++
int32_t andi(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] & curr->arg_3;
    return 1;
}

// Emulates bitwise OR between a register and a 16-bit signed int literal.
// $t = $s | I;    PC++
int32_t ori(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] | curr->arg_3;
    return 1;
}

// Loads a signed integer literal into the upper 16-bits of a register.
// $t = I << 16;    PC++
int32_t lui(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = curr->arg_3 << 16;
    return 1;
}

// Emulates system calls for a MIPS system.
// Performs a formatted print (stdout) or program exit.
int32_t syscall(int32_t *reg, instr_t *curr) {
    switch (reg[V0]) {
        case PRINT_INT:
            printf("%d", reg[A0]);
            break;

        case EXIT_H:
            return INSTRUCTION_EXIT;

        case PRINT_CHAR:
            putchar(reg[A0]);
            break;

        default:
            printf("Unknown system call: %d\n", reg[V0]);
            return INSTRUCTION_EXIT;
    }
    return 1;
}