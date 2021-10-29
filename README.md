# A very simple emulator for a subset of the MIPS32 instruction set.

Can be built on Linux with gcc toolchain using
`gcc main.c smips.c instructions.c -o smips`

Emulated instructions:
* ADD
* SUB
* AND
* OR
* SLT
* MUL
* BEQ
* BNE
* ADDI
* SLTI
* ANDI
* ORI
* LUI
* SYSCALL