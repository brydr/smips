#include <stdio.h>
#include "smips.h"
#include "instructions.h"

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