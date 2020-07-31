#include <stdint.h>

/* INT32_T REGISTER ALIASES */
#define V0          2
#define A0          4

/* SYSCALL MODES (V0) */
#define PRINT_INT   1 
#define EXIT_H      10
#define PRINT_CHAR  11

/* Function Prototypes*/
int32_t add(int32_t *reg, instr_t *curr);
int32_t sub(int32_t *reg, instr_t *curr);
int32_t and(int32_t *reg, instr_t *curr);
int32_t or(int32_t *reg, instr_t *curr);
int32_t slt(int32_t *reg, instr_t *curr);
int32_t mul(int32_t *reg, instr_t *curr);
int32_t beq(int32_t *reg, instr_t *curr);
int32_t bne(int32_t *reg, instr_t *curr);
int32_t addi(int32_t *reg, instr_t *curr);
int32_t slti(int32_t *reg, instr_t *curr);
int32_t andi(int32_t *reg, instr_t *curr);
int32_t ori(int32_t *reg, instr_t *curr);
int32_t lui(int32_t *reg, instr_t *curr);
int32_t syscall(int32_t *reg, instr_t *curr);