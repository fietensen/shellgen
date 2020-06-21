#include <shellgen/program_usage.h>

void print_usage(char **argv)
{
    printf(
        "ShellGen Usage:\n"
        "\t%s -gen -[mode] [argument1 [argument2 [argument3 [...]]]]\n"
        "\nModes:\n"
        "\traw       - outputs raw bytes\n"
        "\trawstring - outputs \\x escaped characters\n"
        "\tasm       - outputs the shellcode in assembly (intel syntax)\n"
        "\nArgument Syntax:\n"
        "\ttype:value\n"
        "Types:\n"
        "\tint32\n"
        "\tchar\n"
        "\tstring\n"
        "\tregister\n",

        argv[0]
    );
}