/* ........... COMP1521 20T2 --- assignment 2: SMIPS, Simple MIPS .......... */
//               Written by Benjamin Ryder (z5207266), July 2020.
//             See 'main.c' for description of action of program.

/* ----------------------- Instructions Header File ------------------------ */
// * Depends on structs and typedefs in 'smips.h', so it will be included
//   before contents of file.

#include "smips.h"

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

/* INT32_T REGISTER ALIASES */
#define V0          2
#define A0          4

/* SYSCALL MODES (V0) */
#define PRINT_INT   1 
#define EXIT_H      10
#define PRINT_CHAR  11

/* Function Prototypes*/

// Functions modify values of reg[D] (R-type) or reg[T] (I-type) or neither,
// based on some operation between reg[S] and reg[T] (R-type), or 
// between reg[S] and a literal/constant.
// The register indicies and literals are stored within `curr', the referenced
// instruction struct.
// The desired increment to the 'program counter' is returned, which in all 
// cases (except branching instructions) is equal to 1.

int32_t add (int32_t *reg, instr_t *curr);
int32_t sub (int32_t *reg, instr_t *curr);
int32_t and (int32_t *reg, instr_t *curr);
int32_t or  (int32_t *reg, instr_t *curr);
int32_t slt (int32_t *reg, instr_t *curr);
int32_t mul (int32_t *reg, instr_t *curr);

int32_t beq (int32_t *reg, instr_t *curr);
int32_t bne (int32_t *reg, instr_t *curr);
int32_t addi(int32_t *reg, instr_t *curr);
int32_t slti(int32_t *reg, instr_t *curr);
int32_t andi(int32_t *reg, instr_t *curr);
int32_t ori (int32_t *reg, instr_t *curr);
int32_t lui (int32_t *reg, instr_t *curr);

int32_t syscall(int32_t *reg, instr_t *curr);

#endif