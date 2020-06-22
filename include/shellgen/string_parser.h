#ifndef STRING_PARSER_H
#define STRING_PARSER_H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char ***structure;
    int parts;
} WordStructure;

void layout_WordStructure(WordStructure*, char*, int*);
WordStructure parse_stringarray(char*);
void free_WordStructure(WordStructure);

#endif