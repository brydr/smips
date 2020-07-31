#include <stdint.h>

/* BITMASKS 
   for various fields in MIPS32 encoding */
#define BMASK_INSTR  0xfc000000 // 11111100000000000000000000000000
#define BMASK_REG_S  0x3e00000  // 00000011111000000000000000000000
#define BMASK_REG_T  0x1f0000   // 00000000000111110000000000000000
#define BMASK_REG_D  0xf800     // 00000000000000001111100000000000
#define BMASK_LTERAL 0xffff     // 00000000000000001111111111111111
#define BMASK_SUBFLD 0x7ff      // 00000000000000000000011111111111

/* ENCODINGS */
/* Types with zero instruction field */
#define ENC_ADD     0x20 // 00000100000
#define ENC_SUB     0x22 // 00000100010
#define ENC_AND     0x24 // 00000100100
#define ENC_OR      0x25 // 00000100101
#define ENC_SLT     0x2a // 00000000010
/* Types with non-zero instruction fields */
#define ENC_MUL     0x70000002 // 01110000000000000000000000000010
#define ENC_BEQ     0x10000000 // 00010000000000000000000000000000
#define ENC_BNE     0x14000000 // 00010100000000000000000000000000
#define ENC_ADDI    0x20000000 // 00100000000000000000000000000000
#define ENC_SLTI    0x28000000 // 00101000000000000000000000000000
#define ENC_ANDI    0x30000000 // 00110000000000000000000000000000
#define ENC_ORI     0x34000000 // 00110100000000000000000000000000
#define ENC_LUI     0x3C000000 // 00111100000000000000000000000000
#define ENC_SYSCALL 0xc        // 00000000000000000000000000001100

/* IMPLEMENTATION CONSTANTS */
/* Change this if using an instruction (sub)set with different-sized 
   set of return, temp, saved and arg registers */
#define REG_SIZE    26

/* RETURN CONSTANTS */
#define INSTRUCTION_EXIT    0
#define INVALID_INSTR_ENC   -1

/* FORMAT ID CONSTANTS */
#define TYPE_ERR    0x0 // N/A : unknown instruction code
#define TYPE_SYS    0x1 // syscall
#define TYPE_R      0x2 // <instr> $d, $s, $t
#define TYPE_L1     0x3 // <instr> $t, $s, I
#define TYPE_L2     0x4 // <instr> $s, $t, I
#define TYPE_L3     0x5 //   lui   $t, I

/* ALIASES */
typedef uint8_t bool_t;

/* LOCAL STRUCTS */

/* Instruction */
/* Decoded into the following format: 
   - arguments : registers/immediates
   - "name" - for printing
   - "type" - format for printing
   - action - for execution 
*/
typedef struct instruction instr_t;
typedef int32_t (*perform)(int32_t *reg, instr_t *curr);
struct instruction {
    char *name;
    uint8_t type;
    /* 1,2: 5-bit registers */
    uint8_t reg_S;
    uint8_t reg_T;
    /*   3: 5-bit register or 16-bit immediate */
    uint16_t arg_3;
    /* Points to action of instruction*/
    //int32_t (*action)(int32_t *reg, instr_t *self);
    perform action;
};

/* Program */
typedef struct program {
    uint32_t n_lines;
    /* Array of pointers to instructions (of non-constant length) */
    instr_t **code;
} program_t;

/* FUNCTION PROTOTYPES */

/* Parses over file to determine length and hence necessary allocation */
int get_program_length(FILE *file_pointer);

/* Simple function for checking if c is any 0..9 or a..f or A..F */
bool_t is_hex_val(char c);

/*  Takes a MIPS32 instruction and converts it into a struct containing:
    - up to 3 params/args, with at most one 16-bit signed immediate,
    - the instruction's assembly language layout as a macro constant,
    - the instruction's action as a pointer to a function, and
    - the instruction name as a pointer to a string literal 
    TYPE_ERR is stored in `type' field and returned if instruction doesn't match
    with any in defined subset. */
instr_t *decode_instruction(uint32_t num);

/*  Iterates through line-separated ASCII file containing MIPS32 instructions,
    decoding every instruction and converting into an array of "instruction" 
    structs.
    `file_path' only used for printing errors when unknown instruction 
    encountered. */
program_t *decode_program(FILE *file_ptr, char *file_path);

/*  Prints a "program" struct to stdout. */
void print_program(program_t *mips_program);

/*  Executes a "program" struct */
void execute_program(int32_t *reg, program_t *mips_program);

/*  Scans a "register" (REG_SIZE int array) and prints non-zero values to stdout */
void print_reg_changes(int32_t *reg);

/*  Frees memory allocated for a "program" struct */
void free_program(program_t *mips_program);