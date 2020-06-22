#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define ARGUMENT_MODE_x86_64 "x64"
#define ARGUMENT_MODE_x86_64_2 "x86_64"
#define ARGUMENT_MODE_x86 "x86"
#define ARGUMENT_MODE_GENERATE "generate"
#define ARGUMENT_MODE_GENERATE_2 "gen"
#define ARGUMENT_OMODE_RAW "raw"
#define ARGUMENT_OMODE_HEXESC "rawstring"
#define ARGUMENT_OMODE_ASM "asm"
#define ARGUMENT_ENDIANNESS_LITTLE "little"
#define ARGUMENT_ENDIANNESS_BIG "big"

#define MODE_NONE 0
#define MODE_GENERATE 1

#define TYPEID_STRING "string"
#define TYPEID_INT32 "int32"
#define TYPEID_REGISTER "register"
#define TYPEID_CHAR "char"

#define ENDIANNESS_NONE 0
#define ENDIANNESS_LITTLE 1
#define ENDIANNESS_BIG 2

#define TYPE_CHAR 0
#define TYPE_INT32 1
#define TYPE_REGISTER 2
#define TYPE_STRING 3

#define OMODE_NONE 0
#define OMODE_RAW 1
#define OMODE_HEXESC 2
#define OMODE_ASM 3

typedef struct {
    unsigned char type;
    //struct arg_string _str;
    char *_str;
    int _int32;
    int _reg;
    char _char;
} program_argument;

typedef struct {
    bool x64;
    char endianness;
    unsigned int mode;
    unsigned int output;
    int nArgs;
    program_argument args[7];
} program_arguments;

extern char *registers[];
extern program_arguments _program_arguments;
program_arguments *get_pargs(int, char**);

#endif