#include <shellgen/argument_parser.h>

program_arguments _program_arguments;
char *registers[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", \
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "r8", "r9", "r10", \
    "r11", "r12", "r13", "r14", "r15", 0};

int _getreg(char *regname) {
    int i = 0;
    while (registers[i]) {
        if (strcmp(regname, registers[i]) == 0)
            break;
        i++;
    }
    if (i*sizeof(void*) == sizeof(registers)-sizeof(void*))
        i = -1;
    return i;
}

char *stolower(char *string)
{
    for (unsigned int i=0;string[i];i++)
        string[i] = tolower(string[i]);
    return string;
}

bool _chkint(char *string)
{
    if (strncmp(stolower(string), "0x", 2) == 0 && strlen(string) > 2) {
        for (unsigned int i=2;string[i];i++) {
            if (!((string[i] >= 97 && string[i] <= 122) || (string[i] >= 48 && string[i] <= 57)))
                return false;
        }
    } else {
        for (unsigned int i=0;string[i];i++) {
            if (!(string[i] >= 48 && string[i] <= 57))
                return false;
        }
    }
    return true;
}

#define RFALSE {free(argument);return false;}
#define RTRUE {free(argument);return true;}

bool _isarg(char *arg)
{
    char *argument = malloc(strlen(arg)+1);
    memset(argument, 0, strlen(arg)+1);
    strcpy(argument, arg);

    char *s_ptr = strrchr(argument, ':');
    if (s_ptr == NULL)
        return false;
    
    int s_len = s_ptr-argument;
    s_ptr++;

    if (s_len == strlen(TYPEID_CHAR) && strncmp(argument, TYPEID_CHAR, strlen(TYPEID_CHAR)) == 0) {
        if (strlen(s_ptr) != 1)
            RFALSE;
        RTRUE;
    } else if (s_len == strlen(TYPEID_INT32) && strncmp(argument, TYPEID_INT32, strlen(TYPEID_INT32)) == 0) {
        if (!_chkint(s_ptr))
            RFALSE;
        RTRUE;
    } else if (s_len == strlen(TYPEID_REGISTER) && strncmp(argument, TYPEID_REGISTER, strlen(TYPEID_REGISTER)) == 0) {
        if (_getreg(stolower(s_ptr)) == -1)
            RFALSE;
        RTRUE;
    } else if (s_len == strlen(TYPEID_STRING) && strncmp(argument, TYPEID_STRING, strlen(TYPEID_STRING)) == 0) {
        RTRUE;
    } else {
        RFALSE;
    }
}
#undef RFALSE
#undef RTRUE


program_argument _parsearg(char *argument)
{
    program_argument arg;

    char *s_ptr = strrchr(argument, ':');
    int s_len = s_ptr-argument;
    s_ptr++;

    if (s_len == strlen(TYPEID_CHAR) && strncmp(argument, TYPEID_CHAR, strlen(TYPEID_CHAR)) == 0) {
        arg.type = 0;
        arg._char = s_ptr[0];
    } else if (s_len == strlen(TYPEID_INT32) && strncmp(argument, TYPEID_INT32, strlen(TYPEID_INT32)) == 0) {
        arg.type = 1;
        if (strncmp(s_ptr, "0x", 2) == 0)
            sscanf(s_ptr, "%x", &arg._int32);
        else
            sscanf(s_ptr, "%d", &arg._int32);
    } else if (s_len == strlen(TYPEID_REGISTER) && strncmp(argument, TYPEID_REGISTER, strlen(TYPEID_REGISTER)) == 0) {
        arg.type = 2;
        arg._reg = _getreg(stolower(s_ptr));
    } else if (s_len == strlen(TYPEID_STRING) && strncmp(argument, TYPEID_STRING, strlen(TYPEID_STRING)) == 0) {
        arg.type = 3;
        arg._str = s_ptr;
    }

    return arg;
}

bool validate_arch(program_arguments *pArgs)
{
    for (int i=0;i<pArgs->nArgs;i++) {
        if (pArgs->args[i].type == TYPE_REGISTER) {
            if (pArgs->args[i]._reg >= _getreg("rax") && !pArgs->x64) {
                printf("[ERROR]: Cannot use register [%s] in 32 bit mode.\n", registers[pArgs->args[i]._reg]);
                return false;
            } else if (pArgs->args[i]._reg < _getreg("rax") && pArgs->x64) {
                printf("[ERROR]: Cannot use register [%s] in 64 bit mode.\n", registers[pArgs->args[i]._reg]);
                return false;
            }
        }
    }
    return true;
}

program_arguments *get_pargs(int argc, char **argv)
{    
    // clearing the argument structure
    _program_arguments.nArgs = 0;
    _program_arguments.x64 = false;
    _program_arguments.mode = MODE_NONE;
    _program_arguments.output = OMODE_NONE;

    // mapping arguments
    for (int i=1;i<argc;i++) {
        if (strncmp(argv[i], "-", 1) == 0 && strlen(argv[i]) > 1) {
            if (strcmp(argv[i]+1, ARGUMENT_MODE_x86_64) == 0 || strcmp(argv[i]+1, ARGUMENT_MODE_x86_64_2) == 0) {
                 _program_arguments.x64 = true;
            } else if (strcmp(argv[i]+1, ARGUMENT_MODE_x86) == 0) {
                _program_arguments.x64 = false;
            } else if (strcmp(argv[i]+1, ARGUMENT_MODE_GENERATE) == 0 || strcmp(argv[i]+1, ARGUMENT_MODE_GENERATE_2) == 0) {
                _program_arguments.mode = MODE_GENERATE;
            } else if (strcmp(argv[i]+1, ARGUMENT_OMODE_ASM) == 0) {
                _program_arguments.output = OMODE_ASM;
            } else if (strcmp(argv[i]+1, ARGUMENT_OMODE_HEXESC) == 0) {
                _program_arguments.output = OMODE_HEXESC;
            } else if (strcmp(argv[i]+1, ARGUMENT_OMODE_RAW) == 0) {
                _program_arguments.output = OMODE_RAW;
            } else {
                printf("[ERROR]: Invalid argument supplied. \"%s\" (Argument %d)\n", argv[i], i);
                return NULL;
            }
        } else if (_isarg(argv[i])) {
            if ((_program_arguments.nArgs < 7 && !_program_arguments.x64) || (_program_arguments.nArgs < 8 || _program_arguments.x64)) {
                _program_arguments.args[_program_arguments.nArgs] = _parsearg(argv[i]);
                _program_arguments.nArgs++;
            } else {
                printf("[ERROR]: Too many parameters passed. \"%s\" (Parameter %d)\n", argv[i], _program_arguments.nArgs+1);
                return NULL;
            }
        } else {
            printf("[ERROR]: Invalid argument supplied. \"%s\" (Argument %d)\n", argv[i], i);
            return NULL;
        }
    }

    if (!validate_arch(&_program_arguments)) return NULL;
    return &_program_arguments;
}