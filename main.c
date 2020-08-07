/* ........... COMP1521 20T2 --- assignment 2: SMIPS, Simple MIPS ........... */
//               Written by Benjamin Ryder (z5207266), July 2020.
// 
// * An emulator for a small, simple subset of the MIPS instruction set
//   architecture.
// * Input: Expects encoded MIPS instructions in hexademical in an 
//   ASCII-encoded file.
// * Output: Returns assembly representation of MIPS instructions, and emulates
//   their action with resulting stdout. Modified registers are printed after
//   execution.
// * Primarily works through converting encoded instructions into an array of 
//   structs which store all relevant characteristics for printing/performing.

#include "smips.h"

int main (int argc, char *argv[]) {
    char *in_file_path = argv[1];
    FILE *file_pointer = fopen(in_file_path, "r");
    // Use an array to emulate register indicies
    int32_t reg[REG_SIZE] = {0};

    // Decode program into `instr_t' structs, and pass file name just in case an
    // invalid instruction is encountered.
    program_t *my_program = decode_program(file_pointer, in_file_path);

    // Print out text assembly for MIPS program
    printf("Program\n");
    print_program(my_program);

    // Run MIPS program with emulated syscalls, including stdout writes
    printf("Output\n");
    execute_program(reg, my_program);

    // Scan for any registers altered by execution
    printf("Registers After Execution\n");
    print_reg_changes(reg);

    // Free memory allocation
    free_program(my_program);
    return 0;
}