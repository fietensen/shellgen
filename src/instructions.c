#include <shellgen/instructions.h>

struct sRegister lRegisters[2][7] = {
        {
            {.name="eax", .pop="\x58", .xor="\x31\xc0", .push="\x50", .movs="\x89\xe0"},
            {.name="ebx", .pop="\x5B", .xor="\x31\xdb", .push="\x53", .movs="\x89\xe3"},
            {.name="ecx", .pop="\x59", .xor="\x31\xc9", .push="\x51", .movs="\x89\xe1"},
            {.name="edx", .pop="\x5A", .xor="\x31\xd2", .push="\x52", .movs="\x89\xe2"},
            {.name="esi", .pop="\x5E", .xor="\x31\xf6", .push="\x56", .movs="\x89\xe6"},
            {.name="edi", .pop="\x5F", .xor="\x31\xff", .push="\x57", .movs="\x89\xe7"},
            {.name="nil"}
        },
        {
            {.name="rax", .pop="\x58", .xor="\x48\x31\xc0", .movs="\x48\x89\xe0"},
            {.name="rdi", .pop="\x5F", .xor="\x48\x31\xff", .movs="\x48\x89\xe7"},
            {.name="rsi", .pop="\x5E", .xor="\x48\x31\xf6", .movs="\x48\x89\xe6"},
            {.name="rdx", .pop="\x5A", .xor="\x48\x31\xd2", .movs="\x48\x89\xe2"},
            {.name="r10", .pop="\x41\x5A", .xor="\x4d\x31\xd2", .movs="\x49\x89\xe2"},
            {.name="r8", .pop="\x41\x58", .xor="\x4d\x31\xc0", .movs="\x49\x89\xe0"},
            {.name="r9", .pop="\x41\x59", .xor="\x4d\x31\xc9", .movs="\x49\x89\xe1"}
        }
};