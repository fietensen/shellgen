#include <shellgen/program_usage.h>
#include <shellgen/argument_parser.h>
#include <shellgen/argument_packer.h>
#include <stdio.h>
#include <string.h>

char *types[] = {
    "CHAR",
    "INT32",
    "REGISTER",
    "STRING"
};

int main(int argc, char **argv)
{
    if (argc == 1) {
        print_usage(argv);
        return 1;
    } else if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-help") == 0)) {
        print_usage(argv);
        return 0;
    }
    
    program_arguments *pArgs = get_pargs(argc, argv);
    if (pArgs == NULL) {
        return 1;
    } else if (pArgs->mode == MODE_NONE) {
        printf("[ERROR]: No mode specified.\n");
        return 1;
    } else if (pArgs->output == OMODE_NONE) {
        printf("[ERROR]: No output mode specified.\n");
        return 1;
    } else if (pArgs->mode == MODE_GENERATE) {
        for (int i=0;i<pArgs->nArgs;i++) {
            pack_argument(pArgs, i);
        }
        switch (pArgs->output)
        {
        case OMODE_RAW:
            fputs(pArgs->x64 ? "\x0f\x05" : "\xcd\x80", stdout);
            break;
        case OMODE_HEXESC:
            fputs(pArgs->x64 ? "\\x0f\\x05" : "\\xcd\\x80", stdout);
            break;
        case OMODE_ASM:
            fputs(pArgs->x64 ? "syscall\n" : "int 0x80\n", stdout);
            break;
        }
    }

}