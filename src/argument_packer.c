#include <shellgen/argument_packer.h>

// kind of a filthy hack but hey, no malloc where it isn't needed, right? ;D
unsigned int pad_hexc(unsigned char c)
{
    char buf[] = "\x00\x00\x00";
    char output[] = "00\x00\x00";
    sprintf(buf, "%x", c);
    sprintf((strlen(buf) == 1) ? output+1 : output, "%s", buf);
    return *(unsigned int*)output;
}

// check nullbytes
bool has_nullbytes(int n)
{
    return ((((char*)&n)[0] == 0) || (((char*)&n)[1] == 0) || (((char*)&n)[2] == 0) || (((char*)&n)[3] == 0));
}

int gen_xor(int n)
{
    return 0x01010100 + (n % 10) + 1;
}

void pack_argument(program_arguments *pArgs, int i)
{
    // PUSHing the value
    switch (pArgs->args[i].type) {
    case TYPE_CHAR: {
        switch (pArgs->output) {
        case OMODE_RAW: {
            fputc('\x6A', stdout);
            fputc(pArgs->args[i]._char, stdout);
            break;}
        case OMODE_HEXESC: {
            fputs("\\x6a", stdout);
            unsigned int hEsc = pad_hexc(pArgs->args[i]._char);
            fprintf(stdout, "\\x%s", (char*)&hEsc);
            break;}
        case OMODE_ASM: {
            fprintf(stdout, "push '%c'\n", pArgs->args[i]._char);
            break;}
        } break;} 

    case TYPE_INT32: {
        switch (pArgs->output) {
        case OMODE_RAW: {
            if (pArgs->args[i]._int32 == 0) {
                fprintf(stdout, "%s", lRegisters[pArgs->x64][i].xor);
                return;
            }
            fputc('\x68', stdout);
            int argVal = pArgs->args[i]._int32;
            bool nb = false;

            if ((nb = has_nullbytes(argVal))) argVal = gen_xor(argVal);
            fwrite(&argVal, sizeof(int), 1, stdout);
            if (nb) {
                fputs("\x81\x34\x24", stdout);
                int num = (argVal^pArgs->args[i]._int32);
                fwrite(&num, sizeof(int), 1, stdout);
            }
            break;}
        case OMODE_HEXESC: {
            if (pArgs->args[i]._int32 == 0) {
                char *xor = lRegisters[pArgs->x64][i].xor;
                for (int c=0;c<strlen(xor);c++) {
                    unsigned int hEsc = pad_hexc(xor[c]);
                    fprintf(stdout, "\\x%s", (char*)&hEsc);
                }
                return;
            }
            fputs("\\x68", stdout);
            int argVal = pArgs->args[i]._int32;
            bool nb = false;

            if ((nb = has_nullbytes(argVal))) argVal = gen_xor(argVal);
            for (int c=0;c<sizeof(int);c++) {
                unsigned int hEsc = pad_hexc(((char*)&argVal)[c]);
                fprintf(stdout, "\\x%s", (char*)&hEsc);
            }
            if (nb) {
                fputs("\\x81\\x34\\x24", stdout);
                int n = argVal^pArgs->args[i]._int32;
                for (int c=0;c<sizeof(int);c++) {
                    unsigned int hEsc = pad_hexc(((char*)&n)[c]);
                    fprintf(stdout, "\\x%s", (char*)&hEsc);
                }
            }
            break;}
        case OMODE_ASM: {
            if (pArgs->args[i]._int32 == 0) {
                char *regname = lRegisters[pArgs->x64][i].name;
                fprintf(stdout, "xor %s, %s\n", regname, regname);
                return;
            }

            int argVal = pArgs->args[i]._int32;
            bool nb = false;

            if ((nb = has_nullbytes(argVal))) argVal = gen_xor(argVal);

            fprintf(stdout, "push 0x%x\n", argVal);
            if (nb) {
                fprintf(stdout, "xor dword ptr [esp], 0x%x\n", argVal^pArgs->args[i]._int32);
            }

            break;}
        } break;}
    case TYPE_REGISTER: {
        int reg = pArgs->x64 ? pArgs->args[i]._reg-8 : pArgs->args[i]._reg;
        bool rReg = (reg > 5);
        if (pArgs->x64 && rReg) reg -= 6;

        switch (pArgs->output) {
        case OMODE_RAW: {
            if (rReg) {
                fputc(0x41, stdout);
            }
            fputc(0x50+reg, stdout);
            break;}
        case OMODE_HEXESC: {
            if (rReg) {
                fputs("\\x41", stdout);
            }
            unsigned int hEsc = pad_hexc(0x50+reg);
            fprintf(stdout, "\\x%s", (char*)&hEsc);
            break;}
        case OMODE_ASM: {
            fprintf(stdout, "push %s\n", registers[pArgs->args[i]._reg]);
            break;}
        } break;}
    case TYPE_STRING: {
        // parsing the parts into 4 byte pieces
        

        switch (pArgs->output) {
        case OMODE_RAW: {
            break;}
        case OMODE_HEXESC: {

            break;}
        case OMODE_ASM: {

            break;}
        } break;}
    }

    // POPing the value
    switch (pArgs->args[i].type) {
    case TYPE_CHAR: case TYPE_INT32: case TYPE_REGISTER: {
        switch (pArgs->output) {
        case OMODE_RAW: {
            fputs(lRegisters[pArgs->x64][i].pop, stdout);
            break;}
        case OMODE_HEXESC: {
            char *pop = lRegisters[pArgs->x64][i].pop;
            for (int c=0;c<strlen(pop);c++) {
                unsigned int hEsc = pad_hexc(pop[c]);
                fprintf(stdout, "\\x%s", (char*)&hEsc);
            }
            break;}
        case OMODE_ASM: {
            fprintf(stdout, "pop %s\n", lRegisters[pArgs->x64][i].name);
            break;}
        }
        break;}
    }
}