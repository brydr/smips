#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "smips.h"
#include "instructions.h"

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
            printf( "%s:%d invalid instruction code: %d", file_path, index, 
                    instr_code );
            exit(0);
        }

        index++;
    } while (fgets(str_buf, 8 + 2, file_ptr) != NULL);
    return mips_program;
}

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

instr_t *decode_instruction(uint32_t num) {
    instr_t *curr_instruct = malloc (sizeof(struct instruction));

    curr_instruct->reg_S = (num & BMASK_REG_S) >> 21;
    curr_instruct->reg_T = (num & BMASK_REG_T) >> 16;

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

void execute_program(int32_t *reg, program_t *mips_program) {
    uint32_t i = 0;
    while (i < mips_program->n_lines) {
        instr_t *curr = mips_program->code[i];
        int32_t pc_offs = curr->action(reg, curr);
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
    return;
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