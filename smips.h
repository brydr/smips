/* ........... COMP1521 20T2 --- assignment 2: SMIPS, Simple MIPS .......... */
//               Written by Benjamin Ryder (z5207266), July 2020.
//             See 'main.c' for description of action of program.

/* --------------------------- SMIPS Header File --------------------------- */
// * Included via. extension file 'instructions.h' to ensure required types
//   and structs are defined.
// * See implementation file 'smips.c' for description of functions.


#ifndef SMIPS_H
#define SMIPS_H

#include <stdio.h>
#include <stdint.h>


/* ----- ENC. FIELD CONSTANTS ----- */
// BITMASKS : for various fields in MIPS32 encoding
#define BMASK_INSTR  0xfc000000 // 11111100000000000000000000000000
#define BMASK_REG_S  0x3e00000  // 00000011111000000000000000000000
#define BMASK_REG_T  0x1f0000   // 00000000000111110000000000000000
#define BMASK_REG_D  0xf800     // 00000000000000001111100000000000
#define BMASK_LTERAL 0xffff     // 00000000000000001111111111111111
#define BMASK_SUBFLD 0x7ff      // 00000000000000000000011111111111

/* ----- ENCODING CONSTANTS ----- */
// Types with zero instruction field
#define ENC_ADD     0x20 // 00000100000
#define ENC_SUB     0x22 // 00000100010
#define ENC_AND     0x24 // 00000100100
#define ENC_OR      0x25 // 00000100101
#define ENC_SLT     0x2a // 00000000010
// Types with non-zero instruction fields
#define ENC_MUL     0x70000002 // 01110000000000000000000000000010
#define ENC_BEQ     0x10000000 // 00010000000000000000000000000000
#define ENC_BNE     0x14000000 // 00010100000000000000000000000000
#define ENC_ADDI    0x20000000 // 00100000000000000000000000000000
#define ENC_SLTI    0x28000000 // 00101000000000000000000000000000
#define ENC_ANDI    0x30000000 // 00110000000000000000000000000000
#define ENC_ORI     0x34000000 // 00110100000000000000000000000000
#define ENC_LUI     0x3C000000 // 00111100000000000000000000000000

#define ENC_SYSCALL 0xc        // 00000000000000000000000000001100

/* ----- IMPLEMENTATION CONSTANTS ----- */
// Change this if using an instruction (sub)set with different-sized 
// set of return, temp, saved and arg registers

#define REG_SIZE    26

/* ----- RETURN CONSTANTS ----- */
#define INSTRUCTION_EXIT    0

/* ----- FORMAT ID CONSTANTS  ----- */
#define TYPE_ERR    0x0 // N/A : unknown instruction code
#define TYPE_SYS    0x1 // syscall
#define TYPE_R      0x2 // <instr> $d, $s, $t
#define TYPE_L1     0x3 // <instr> $t, $s, I
#define TYPE_L2     0x4 // <instr> $s, $t, I
#define TYPE_L3     0x5 //   lui   $t, I

/* ----- LOCAL STRUCTS ----- */

// INSTRUCTION: Decoded instruction containing: 
// - "arguments" : registers/literal
// - name        : for printing
// - type        : format for printing
// - action      : for execution 
//
typedef struct instruction instr_t;
typedef int32_t (*perform)(int32_t *reg, instr_t *curr);
struct instruction {
    // Pointer to string literal for printing instruction's name
    char *name;
    // #define'd constant for printing format
    uint8_t type;
    // 5-bit registers (x2)
    uint8_t reg_S;
    uint8_t reg_T;
    // 5-bit register OR 16-bit signed literal
    uint16_t arg_3;
    // Pointer to which function emulates this instruction 
    perform action;
};

// PROGRAM: A variable-length array of instructions
//
typedef struct program {
    uint32_t n_lines;
    // Array of instr_t
    instr_t *code;
} program_t;

/* ----- FUNCTION PROTOTYPES ----- */

// all descriptions in implementation file: smips.c

int get_program_length(FILE *file_pointer);
uint8_t is_valid_char(char c);
instr_t decode_instruction(uint32_t num);
program_t *decode_program(FILE *file_ptr, char *file_path);

void print_program(program_t *mips_program);
void execute_program(int32_t *reg, program_t *mips_program);
void print_reg_changes(int32_t *reg);

void free_program(program_t *mips_program);

#endif