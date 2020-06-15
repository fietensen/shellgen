#include <shellgen/program_usage.h>

void print_usage()
{
    printf(
        "ShellGen Usage:\n"
        "\t-gen [argument1 [argument2 [argument3 [...]]]]\n"
        "\nArgument Syntax:\n"
        "\ttype:value\n"
        "Types:\n"
        "\tint32\n"
        "\tchar\n"
        "\tstring\n"
        "\tregister\n"
    );
}