/* ........... COMP1521 20T2 --- assignment 2: SMIPS, Simple MIPS ........... */
//               Written by Benjamin Ryder (z5207266), July 2020.
//             See 'main.c' for description of action of program.

/* ---------------------- SMIPS Implementation File ------------------------  */
// * Implements all major functions which print/execute MIPS instructions after
//   first decoding and translating them into `instruction' structs (as a 
//   single `program' array) 
// * instructions.h and associated .c file required for implementing the
//   specific action of each instruction on "registers" (an integer array). 


// #include <stdio.h>       // via. instructions.h->smips.h
// #include <stdint.h>      // via. instructions.h->smips.h
#include <stdlib.h>
#include <assert.h>
// #include "smips.h"       // via. instructions.h
#include "instructions.h"

/* Converts input file to a "program" - an array of "instruction" struct *s -
   by calling decode_instruction() on each encoded instruction.
   `file_path' only used for printing errors when unknown instruction 
   encountered. */
program_t *decode_program(FILE *file_ptr, char *file_path) {
    /* Using get_program_length to pre-determine length of allocation required
       for program, and determine if input is valid. */
    program_t *mips_program = malloc(sizeof(struct program));
    mips_program->n_lines = get_program_length(file_ptr);

    // Zero-length returned if invalid characters are detected
    assert(mips_program->n_lines > 0);
    
    mips_program->code = calloc(mips_program->n_lines, sizeof(instr_t*));
    
    uint32_t index = 0;
    char str_buf[8 + 2] = {0};
    fgets(str_buf, 8 + 2, file_ptr);

    // Iterate through every line.
    // A standard while(){} loop would fail to decode a single-line file.
    do {
        // Skip empty lines
        if (str_buf[0] == '\n') {
            continue;
        }
        uint32_t num = (uint32_t)strtol(str_buf, (char **)0 , 16);
        
        // Convert num to (pointer to m'allocated) `instruction' struct
        mips_program->code[index] = decode_instruction(num);
        
        /* Expecting TYPE_ERR to be assigned to `type' field if decoded 
           instruction doesn't match any of the implemented opcodes */
        if (mips_program->code[index]->type == TYPE_ERR) {
            uint8_t instr_code = (num & BMASK_INSTR) >> 26;
            printf( "%s:%d invalid instruction code: %d\n", file_path, index, 
                    instr_code );
            exit(0);
        }
        index++;
    } while (fgets(str_buf, 8 + 2, file_ptr) != NULL);

    return mips_program;
}

/* Parses over file to determine length and hence necessary allocation */
int get_program_length(FILE *file_pointer) {
    int new_lines = 0;
    int curr_char = fgetc(file_pointer);
    int last_char = '\n';
    while (curr_char != EOF) {
        if (!is_valid_char(curr_char)) {
            printf("Error: Input is not pure hexadecimal.\n");
            return 0;
        }
        // Consecutive line-breaks implies an empty line;
        // we will intentionally exclude these.
        if (curr_char == '\n' && last_char == '\n') {
            new_lines--;
        }
        new_lines += (curr_char == '\n');
        last_char = curr_char;
        curr_char = fgetc(file_pointer);
    }
    // If last line is empty we will ignore it.
    if (curr_char == EOF && last_char == '\n') {
        new_lines--;
    }
    // Restore position of file descriptor/pointer.
    rewind(file_pointer);
    return new_lines + 1;
}

/* Helper function for get_program_length().
   Determines if char is expected input: either a hexadecimal or a linebreak. */
uint8_t is_valid_char(char c) {
    // Branchless (logic expression) somehow produces more instructions
    // when compiled to x86
    if (c == '\n') {
        return 1;
    }
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
    - up to 3 params/args, with at most one 16-bit signed literal,
    - type:     the format of the instruction in assembly as a macro constant,
    - name:     the instruction name as a string (pointer to a literal),
    - action:   the action of the instruction as a pointer to a function 
                (see instruction.h).
    TYPE_ERR is stored in `type' field and returned if instruction doesn't match
    with any in defined subset. */
instr_t *decode_instruction(uint32_t num) {
    instr_t *curr_instruct = malloc (sizeof(struct instruction));

    curr_instruct->reg_S = (num & BMASK_REG_S) >> 21;
    curr_instruct->reg_T = (num & BMASK_REG_T) >> 16;

    /* CASE A: Syscall */
    if (num == ENC_SYSCALL) {
        curr_instruct->name = "syscall";
        curr_instruct->action = &syscall;
        curr_instruct->type = TYPE_SYS;
        curr_instruct->arg_3 = 0;
        return curr_instruct;
    }

    /* CASE B: I-type instructions */
    uint32_t num_masked = num & BMASK_INSTR;
    curr_instruct->type = TYPE_L1;
    curr_instruct->arg_3 = num & BMASK_LTERAL;
    //  We will now use a switch..case (with an intermission to adjust our mask)
    //  to match our instruction field with a #define'd constant. 
    switch (num_masked) {
        // R-type instructions (exc. MUL) have zero instruction field
        case 0x0:
            break;

        case ENC_BEQ:
            // By using string literals there will be no duplication of strings
            // in memory for repeat instructions of same type.
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
            // If nothing matched, then we'll change the mask and test R-types
            break;
    }
    /* CASE C: R-type instructions */
    //  We need to adjust our mask to read the subfield too
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
            // If reached, it means we've found no matching instruction
            // encodings after checking both instruction field and subfield
            curr_instruct->type = TYPE_ERR;
            break;
    }
    return curr_instruct;
}

/*  Prints a "program" struct to stdout. */
void print_program(program_t *mips_program) {
    uint32_t n_lines = mips_program->n_lines;
    for (uint32_t i = 0; i < n_lines; i++) {
        instr_t *curr = mips_program->code[i];
        /* Instruction subset prints in five different formats */
        switch (curr->type) {
            case TYPE_SYS:
                printf("%3d: %s\n", i, curr->name);
                break;

            case TYPE_R:
                printf("%3d: %-4s $%d, $%d, $%d\n", i, curr->name, curr->arg_3,
                       curr->reg_S, curr->reg_T);
                break;

            case TYPE_L1:
                printf("%3d: %-4s $%d, $%d, %d\n", i, curr->name, curr->reg_T, 
                       curr->reg_S, (int16_t)curr->arg_3);
                break;

            case TYPE_L2:
                printf("%3d: %-4s $%d, $%d, %d\n", i, curr->name, curr->reg_S, 
                       curr->reg_T, (int16_t)curr->arg_3);
                break;

            case TYPE_L3:
                printf("%3d: %-4s $%d, %d\n", i, curr->name, curr->reg_T, 
                       (int16_t)curr->arg_3);
                break;

            default:
                assert(0);
                break;
        }
    }
    return;
}

/*  Executes a `program' struct by iterating through all `instruction' structs 
    and calling the function pointed to by the `action' field */
void execute_program(int32_t *reg, program_t *mips_program) {
    uint32_t i = 0;
    while (i < mips_program->n_lines) {
        instr_t *curr = mips_program->code[i];
        int32_t pc_offs = curr->action(reg, curr);
        // Exit triggered by code 10 or unknown syscall
        if (pc_offs == INSTRUCTION_EXIT) {
            return;
        }
        // Quick and dirty way of ensuring $0 is unchanged
        reg[0] = 0;
        i += pc_offs;
    }
    return;
}

/* Checks through a "register" (REG_SIZE-sized int array) and prints any 
   non-zero values */
void print_reg_changes(int32_t *reg) {
    // Iterate through modifiable registers and print non-zero values
    // Skipping reg[0] since 0 is re-assigned to this reg after every instr.
    for (int i = 1; i < REG_SIZE; i++) {
        if (reg[i] != 0) {
            printf("$%-2d = %d\n", i, reg[i]);
        }
    }
    return;
}

/*  Frees memory allocatation for a "program" struct and its components */
void free_program(program_t *mips_program) {
    // Iterate through each line of the instructions
    for (uint32_t i = 0; i < mips_program->n_lines; i++) {
        // Free data attached to instruction
        free(mips_program->code[i]);
    }
    // Free (array of) pointers to instructions
    free(mips_program->code);
    // Free pointer to above array & its stored length
    free(mips_program);
    return;
}