#include <shellgen/program_usage.h>
#include <shellgen/argument_parser.h>
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
        print_usage();
        return 1;
    } else if ((strcmp(argv[1], "--help") == 0)) {
        print_usage();
        return 0;
    }
    
    program_arguments *pArgs = get_pargs(argc, argv);
    if (pArgs == NULL) {
        return 1;
    } else if (pArgs->mode == MODE_NONE) {
        printf("[ERROR]: No mode specified.\n");
        return 1;
    } else if (pArgs->mode == MODE_GENERATE) {
        for (int i=0;i<pArgs->nArgs;i++) {
            printf("Argument %d is of type %s.\n", i, types[pArgs->args[i].type]);
        }
    }

}