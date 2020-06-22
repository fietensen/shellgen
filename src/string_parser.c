#include <shellgen/string_parser.h>

void free_WordStructure(WordStructure ws)
{
    for (int i=0;i<ws.parts;i++) {
        char **part = ws.structure[i];
        int j = 0;
        while (part[j] != NULL) free(part[j]);
        free(part);
    }
    free(ws.structure);
}

WordStructure parse_stringarray(char *string)
{
    WordStructure ws;
    
    ws.parts = 0;
    ws.structure = malloc((ws.parts+1)*sizeof(char**));
    int wsz = 0;
    ws.structure[ws.parts] = malloc((wsz+1)*sizeof(char*));
    ws.structure[ws.parts][wsz] = NULL;

    char buffer[5];
    memset(buffer, 0, 5*sizeof(char));
    int i = 0;
    int windex = 0;

    for (;i<strlen(string);i++) {
        char character = string[i];

        if (character == ',') {
            if (windex) {
                layout_WordStructure(&ws, buffer, &wsz);
            }
            wsz = 0;
            ws.parts++;
            ws.structure = realloc(ws.structure, (ws.parts+1)*sizeof(char**));
            ws.structure[ws.parts] = malloc((wsz+1)*sizeof(char*));
            ws.structure[ws.parts][wsz] = NULL;
            windex = 0;
            continue;
        }

        buffer[windex] = character;
        if (windex == 3) {
            layout_WordStructure(&ws, buffer, &wsz);
        }
        windex = (windex+1)%4;
    }
    
    if ((i-ws.parts)%4) {
        layout_WordStructure(&ws, buffer, &wsz);
    }

    ws.parts++;
    return ws;
}

void layout_WordStructure(WordStructure *ws, char *buffer, int *wsz)
{
    ws->structure[ws->parts][*wsz] = malloc(5*sizeof(char)); // creating element in 4b char* array for buffer
    memset(ws->structure[ws->parts][*wsz], 0, 5*sizeof(char)); // zeroing element
    memcpy(ws->structure[ws->parts][*wsz], buffer, 4*sizeof(char)); // copying over the element
    *wsz = *wsz+1;
    ws->structure[ws->parts] = realloc(ws->structure[ws->parts], (*wsz + 1)*sizeof(char*)); // widening 4b char* array
    ws->structure[ws->parts][*wsz] = NULL; // setting new element to NULL (end)
    memset(buffer, 0, 5*sizeof(char));
}