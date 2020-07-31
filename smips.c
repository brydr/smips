#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* Bitmasks for various fields, converted to hex */
#define BMASK_INSTR  0xfc000000 // 11111100000000000000000000000000
#define BMASK_REG_S  0x3e00000  // 00000011111000000000000000000000
#define BMASK_REG_T  0x1f0000   // 00000000000111110000000000000000
#define BMASK_REG_D  0xf800     // 00000000000000001111100000000000
#define BMASK_LTERAL 0xffff     // 00000000000000001111111111111111
#define BMASK_SUBFLD 0x7ff      // 00000000000000000000011111111111

/* Encodings */
/* Types with zero instruction field */
#define ENC_ADD 0x20 // 00000100000
#define ENC_SUB 0x22 // 00000100010
#define ENC_AND 0x24 // 00000100100
#define ENC_OR  0x25 // 00000100101
#define ENC_SLT 0x2a // 00000000010
/* Types with non-zero instruction fields */
#define ENC_MUL  0x70000002 // 01110000000000000000000000000010
#define ENC_BEQ  0x10000000 // 00010000000000000000000000000000
#define ENC_BNE  0x14000000 // 00010100000000000000000000000000
#define ENC_ADDI 0x20000000 // 00100000000000000000000000000000
#define ENC_SLTI 0x28000000 // 00101000000000000000000000000000
#define ENC_ANDI 0x30000000 // 00110000000000000000000000000000
#define ENC_ORI  0x34000000 // 00110100000000000000000000000000
#define ENC_LUI  0x3C000000 // 00111100000000000000000000000000
#define ENC_SYSCALL 0xc     // 00000000000000000000000000001100

#define INSTRUCTION_EXIT 0
#define INVALID_INSTR_ENC -1
#define REG_SIZE    26

#define TYPE_ERR    0x0 // N/A : unknown instruction code
#define TYPE_SYS    0x1 // syscall
#define TYPE_R      0x2 // <instr> $d, $s, $t
#define TYPE_L1     0x3 // <instr> $t, $s, I
#define TYPE_L2     0x4 // <instr> $s, $t, I
#define TYPE_L3     0x5 //   lui   $t, I

/* int32 Register Aliases */
#define V0          2
#define A0          4

/* Syscall Modes */
#define PRINT_INT   1 
#define EXIT_H      10
#define PRINT_CHAR  11

/* Using 'bool' as an alias to convey intention */
typedef uint8_t bool_t;

/* Instructions will be decoded into the following format: 
   - arguments : registers/immediates
   - "name" - for printing
   - "type" - format for printing
   - action - for execution */
typedef int32_t (*perform)(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);

typedef struct instruction {
    char *name;
    uint8_t type;
    /* 1,2: 5-bit registers */
    uint8_t arg_S;
    uint8_t arg_T;
    /*   3: 5-bit register or 16-bit immediate */
    uint16_t arg_3;
    /* Points to action of instruction*/
    perform action;
} instr_t;

typedef struct program {
    uint32_t n_lines;
    /* Array of pointers to instructions (of non-constant length) */
    instr_t **code;
} program_t;

int get_program_length(FILE *file_pointer);
bool_t is_hex_val(char c);
instr_t *decode_instruction(uint32_t num);
program_t *decode_program(FILE *file_ptr, char *file_path);
void print_program(program_t *mips_program);
void execute_program(int32_t *reg, program_t *mips_program);
void print_reg_changes(int32_t *reg);
void free_program(program_t *mips_program);

int32_t add(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t sub(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t and(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t or(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t slt(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t mul(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t beq(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t bne(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t addi(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t slti(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t andi(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t ori(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t lui(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);
int32_t syscall(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3);


int main (int argc, char *argv[]) {
    char *in_file_path = argv[1];
    FILE *file_pointer = fopen(in_file_path, "r");
    int32_t reg[REG_SIZE] = {0};
    //int len = get_program_length(file_pointer);
    //printf("%d", len);
    program_t *my_program = decode_program(file_pointer, in_file_path);
    printf("Program\n");
    print_program(my_program);
    printf("Output\n");
    execute_program(reg, my_program);
    printf("Registers After Execution\n");
    print_reg_changes(reg);
    free_program(my_program);
    return 0;
}

/*  Iterates through line-separated ASCII file containing MIPS32 instructions,
    encoded as hexadecimal. Decodes every instruction and converts into an array
    of "instruction" structs. 
    `file_path' only used for printing errors when unknown instruction 
    encountered. */
program_t *decode_program(FILE *file_ptr, char *file_path) {
    char str_buf[8 + 2] = {0};
    /* Using get_program_length to check program char-by-char reliably
       determine if input is valid. */
    program_t *mips_program = malloc(sizeof(struct program));
    mips_program->n_lines = get_program_length(file_ptr);
    mips_program->code = calloc(mips_program->n_lines, sizeof(instr_t*));
    uint32_t index = 0;
    fgets(str_buf, 8 + 2, file_ptr);
    /* A while loop would fail to decode a single-instruction file */
    do {
        /*  Intentionally reading in '\n' char from file; else it would trigger
            another exit from fgets */
        uint32_t num = (uint32_t)strtol(str_buf, (char **)0 , 16);
        
        /* Convert num to (pointer to malloc'd) "instruction" struct */
        mips_program->code[index] = decode_instruction(num);
        
        /* If decoded instruction does not match */
        if (mips_program->code[index]->type == TYPE_ERR) {
            uint8_t instr_code = (num & BMASK_INSTR) >> 26;
            printf("%s:%d invalid instruction code: %d", file_path, index, instr_code);
            exit(0);
        }

        index++;
    } while (fgets(str_buf, 8 + 2, file_ptr) != NULL);
    return mips_program;
}

/* Preparses program to determine if all instructions are valid*/
int get_program_length(FILE *file_pointer) {
    int length = 0;
    bool_t curr_line_has_valid = 0;
    char curr_char = (char)fgetc(file_pointer);
    while (curr_char != EOF) {
        if (is_hex_val(curr_char)) {
            /* (1) : 0-9/A-F character means we can count this line */
            curr_line_has_valid = 1;
        } else if (curr_char == '\n' && curr_line_has_valid) {
            /* (2) : Linebreak and valid preceding chars -> +1 valid lines */
            length++;
            curr_line_has_valid = 0;
        } else if (curr_char == '\n') {
            /* (3) : Debug s*/
            printf("Warning: Empty line encountered\n");
            assert(0);
        }
        /* Return a negative value (error) is a non-hex and non-newline char is 
           encountered */
        if ( !is_hex_val(curr_char) && curr_char != '\n' ) {
            return INVALID_INSTR_ENC;
        }
        curr_char = (char)fgetc(file_pointer);
    }
    /* If last line was valid but didn't have a following line-break then we
       should still count it */
    if (curr_char == EOF && curr_line_has_valid) {
        length++;
    }
    /* Restore position of pointer */
    rewind(file_pointer);
    return length;
}

/* Simple function for checking if c is any 0..9 or a..f or A..F */
bool_t is_hex_val(char c) {
    if (c >= '0' && c <= '9') {
        return 1;
    }
    if (c >= 'a' && c <= 'f') {
        return 1;
    }
    if (c >= 'A' && c <= 'F') {
        return 1;
    }
    return 0;
}

/*  Takes a MIPS32 instruction and converts it into a struct containing:
    - up to 3 params/args, with at most one 16-bit signed immediate,
    - the instruction's assembly language layout as a macro constant,
    - the instruction's action as a pointer to a function, and
    - the instruction name as a pointer to a string literal 
    TYPE_ERR is stored in `type' field and returned if instruction doesn't match
    with any in defined subset. */
instr_t *decode_instruction(uint32_t num) {
    instr_t *curr_instruct = malloc (sizeof(struct instruction));

    curr_instruct->arg_S = (num & BMASK_REG_S) >> 21;
    curr_instruct->arg_T = (num & BMASK_REG_T) >> 16;

    /* Case A: Syscall */
    if (num == ENC_SYSCALL) {
        curr_instruct->name = "syscall";
        curr_instruct->action = &syscall;
        curr_instruct->type = TYPE_SYS;
        curr_instruct->arg_3 = 0;
        return curr_instruct;
    }    
    /* Case B: I-type instructions */
    uint32_t num_masked = num & BMASK_INSTR;
    curr_instruct->type = TYPE_L1;
    curr_instruct->arg_3 = num & BMASK_LTERAL;
    switch (num_masked) {
        /* R-type instructions (exc. MUL) have zero instruction field */
        case 0x0:    
            break;

        case ENC_BEQ:
            /* By using string literals there will be no duplication of strings
               in memory for repeat instructions of same type. */
            curr_instruct->name = "beq";
            curr_instruct->action = &beq;
            curr_instruct->type = TYPE_L2;
            return curr_instruct;

        case ENC_BNE:
            curr_instruct->name = "bne";
            curr_instruct->action = &bne;
            curr_instruct->type = TYPE_L2;
            return curr_instruct;

        case ENC_ADDI:
            curr_instruct->name = "addi";
            curr_instruct->action = &addi;
            return curr_instruct;

        case ENC_SLTI:
            curr_instruct->name = "slti";
            curr_instruct->action = &slti;
            return curr_instruct;

        case ENC_ANDI:
            curr_instruct->name = "andi";
            curr_instruct->action = &andi;
            return curr_instruct;

        case ENC_ORI:
            curr_instruct->name = "ori";
            curr_instruct->action = &ori;
            return curr_instruct;

        case ENC_LUI:
            curr_instruct->name = "lui";
            curr_instruct->action = &lui;
            curr_instruct->type = TYPE_L3;
            return curr_instruct;

        default:
            break;
    }
    /* Case C: R-type instructions */
    /* We need to adjust our mask to read the subfield. */
    num_masked = num & (BMASK_INSTR  | BMASK_SUBFLD);
    curr_instruct->arg_3 = (num & BMASK_REG_D) >> 11;
    curr_instruct->type = TYPE_R;
    switch (num_masked) {
        case ENC_ADD:
            curr_instruct->name = "add";
            curr_instruct->action = &add;
            return curr_instruct;
        
        case ENC_SUB:
            curr_instruct->name = "sub";
            curr_instruct->action = &sub;
            return curr_instruct;

        case ENC_AND:
            curr_instruct->name = "and";
            curr_instruct->action = &and;
            return curr_instruct;
        
        case ENC_OR:
            curr_instruct->name = "or";
            curr_instruct->action = &or;
            return curr_instruct;

        case ENC_SLT:
            curr_instruct->name = "slt";
            curr_instruct->action = &slt;
            return curr_instruct;

        case ENC_MUL:
            curr_instruct->name = "mul";
            curr_instruct->action = &mul;
            return curr_instruct;

        default:
            /*  If reached, it means we've found no matching instruction
                encodings after checking both instruction field and subfield */
            curr_instruct->type = TYPE_ERR;
            break;
    }
    return curr_instruct;
}

void print_program(program_t *mips_program) {
    uint32_t n_lines = mips_program->n_lines;
    for (uint32_t i = 0; i < n_lines; i++) {
        instr_t *curr = mips_program->code[i];
        switch (curr->type) {
            /* Instruction subset prints in five different formats */
            case TYPE_SYS:
                printf("%3d: %s\n", i, curr->name);
                break;
            case TYPE_R:
                printf("%3d: %-4s $%d, $%d, $%d\n", i, curr->name, curr->arg_3,
                       curr->arg_S, curr->arg_T);
                break;
            case TYPE_L1:
                printf("%3d: %-4s $%d, $%d, %d\n", i, curr->name, curr->arg_T, 
                       curr->arg_S, (int16_t)curr->arg_3);
                break;
            case TYPE_L2:
                printf("%3d: %-4s $%d, $%d, %d\n", i, curr->name, curr->arg_S, 
                       curr->arg_T, (int16_t)curr->arg_3);
                break;
            case TYPE_L3:
                printf("%3d: %-4s $%d, %d\n", i, curr->name, curr->arg_T, 
                   (int16_t)curr->arg_3);
                break;
            default:
                assert(0);
                break;
        }
    }
    return;
}

void execute_program(int32_t *reg, program_t *mips_program) {
    uint32_t i = 0;
    while (i < mips_program->n_lines) {
        instr_t *curr = mips_program->code[i];
        int32_t pc_offs = curr->action( reg, curr->arg_S, curr->arg_T, 
                                        curr->arg_3 );
        if (pc_offs == INSTRUCTION_EXIT) {
            return;
        }
        /* Quick and dirty way of ensuring $0 is unchanged */
        reg[0] = 0;
        i += pc_offs;
    }
    return;
}

void print_reg_changes(int32_t *reg) {
    for (int i = 0; i < REG_SIZE; i++) {
        if (reg[i] != 0) {
            printf("$%-2d = %d\n", i, reg[i]);
        }
    }
}

void free_program(program_t *mips_program) {
    for (uint32_t i = 0; i < mips_program->n_lines; i++) {
        instr_t *curr_instr = mips_program->code[i];
        /* Free data attached to instruction */
        free(curr_instr);
    }
    /* Free (array of) pointers to instructions */
    free(mips_program->code);
    /* Free pointer to above & length of mips_program */
    free(mips_program);
    return;
}

int32_t add(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a3] = reg[a1] + reg[a2];
    return 1;
}
int32_t sub(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a3] = reg[a1] - reg[a2];
    return 1;
}
int32_t and(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a3] = reg[a1] & reg[a2];
    return 1;
}
int32_t or(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a3] = reg[a1] | reg[a2];
    return 1;
}
int32_t slt(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a3] = (reg[a1] < reg[a2]);
    return 1;
}
int32_t mul(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a3] = reg[a1] * reg[a2];
    return 1;
}
int32_t beq(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    /* if (reg[a1] == reg[a2]) {return (int16_t)a3} else {return 1} */
    return 1 + ( (int16_t)a3 - 1 ) * ( reg[a1] == reg[a2] );
}
int32_t bne(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    /* if (reg[a1] != reg[a2]) {return (int16_t)a3} else {return 1} */
    return 1 + ( (int16_t)a3 - 1 ) * ( reg[a1] != reg[a2] );
}
int32_t addi(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a2] = reg[a1] + (int16_t)a3;
    return 1;
}
int32_t slti(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a2] = reg[a1] << a3;
    return 1;
}
int32_t andi(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a2] = reg[a1] & a3;
    return 1;
}
int32_t ori(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a2] = reg[a1] | a3;
    return 1;
}
int32_t lui(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
    reg[a2] = a3 << 16;
    return 1;
}
int32_t syscall(int32_t *reg, uint8_t a1, uint8_t a2, uint16_t a3) {
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