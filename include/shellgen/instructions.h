#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

struct opcode {
    char size;
    char *opcode;
};

struct sRegister {
    char *name;
    char *pop, *push, *xor, *movs;
};

extern struct sRegister lRegisters[2][7];

#endif