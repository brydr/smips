#include <stdio.h>
#include "smips.h"
#include "instructions.h"

int32_t add(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] + reg[curr->reg_T];
    return 1;
}
int32_t sub(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] - reg[curr->reg_T];
    return 1;
}
int32_t and(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] & reg[curr->reg_T];
    return 1;
}
int32_t or(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] | reg[curr->reg_T];
    return 1;
}
int32_t slt(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = (reg[curr->reg_S] < reg[curr->reg_T]);
    return 1;
}
int32_t mul(int32_t *reg, instr_t *curr) {
    reg[curr->arg_3] = reg[curr->reg_S] * reg[curr->reg_T];
    return 1;
}
int32_t beq(int32_t *reg, instr_t *curr) {
    /* if (reg[a1] == reg[a2]) {return (int16_t)a3} else {return 1} */
    uint8_t is_eq  = reg[curr->reg_S] == reg[curr->reg_T];
    return  1 + (is_eq)*((int16_t)curr->arg_3 - 1 );
}
int32_t bne(int32_t *reg, instr_t *curr) {
    /* if (reg[a1] != reg[a2]) {return (int16_t)a3} else {return 1} */
    uint8_t is_neq = reg[curr->reg_S] != reg[curr->reg_T];
    return  1 + (is_neq)*((int16_t)curr->arg_3 - 1 );
}
int32_t addi(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] + (int16_t)curr->arg_3;
    return 1;
}
int32_t slti(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] << curr->arg_3;
    return 1;
}
int32_t andi(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] & curr->arg_3;
    return 1;
}
int32_t ori(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = reg[curr->reg_S] | curr->arg_3;
    return 1;
}
int32_t lui(int32_t *reg, instr_t *curr) {
    reg[curr->reg_T] = curr->arg_3 << 16;
    return 1;
}
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