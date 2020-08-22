#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

struct opcode {
    char size;
    char *opcode;
};

struct sRegister {
    char *name;
    char *pop, *push, *xor, *movs, *mov, *xor11;
};

extern struct sRegister lRegisters[2][7];
extern struct sRegister r11;

#endif