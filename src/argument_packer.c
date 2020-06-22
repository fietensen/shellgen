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

unsigned int set_endianness(unsigned int target, char endianness)
{
    int _val = 1;
    char current_endianness = (((char*)&_val)[0] == 1) ? ENDIANNESS_LITTLE : ENDIANNESS_BIG;

    if (endianness != current_endianness) {
        return (((char*)&target)[0]<<24) | (((char*)&target)[1]<<16) | (((char*)&target)[2]<<8) | (((char*)&target)[3]<<0);
    }
    return target;
}

// check nullbytes
bool has_nullbytes(int n)
{
    return ((((char*)&n)[0] == 0) || (((char*)&n)[1] == 0) || (((char*)&n)[2] == 0) || (((char*)&n)[3] == 0));
}

int gen_xor(int n)
{
    int result = 0;
    for (int i=0;i<4;i++) {
        int c = (n>>(i*8)) & 0xFF;
        result += ((c%10)+1)<<(i*8);
    }
    return result;
}

void pack_argument(program_arguments *pArgs, int i)
{
    // mov dword ptr [ebp-0x12], esp

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
            int otemp = set_endianness(argVal, pArgs->endianness);
            fwrite(&otemp, sizeof(int), 1, stdout);
            if (nb) {
                fputs("\x81\x34\x24", stdout);
                int num = (argVal^pArgs->args[i]._int32);
                otemp = set_endianness(num, pArgs->endianness);
                fwrite(&otemp, sizeof(int), 1, stdout);
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
            int otemp = set_endianness(argVal, pArgs->endianness);
            for (int c=0;c<sizeof(int);c++) {
                unsigned int hEsc = pad_hexc(((char*)&otemp)[c]);
                fprintf(stdout, "\\x%s", (char*)&hEsc);
            }
            if (nb) {
                fputs("\\x81\\x34\\x24", stdout);
                int n = argVal^pArgs->args[i]._int32;
                otemp = set_endianness(n, pArgs->endianness);
                for (int c=0;c<sizeof(int);c++) {
                    unsigned int hEsc = pad_hexc(((char*)&otemp)[c]);
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

            int otemp = set_endianness(argVal, pArgs->endianness);
            fprintf(stdout, "push 0x%x\n", otemp);
            if (nb) {
                otemp = set_endianness(argVal^pArgs->args[i]._int32, pArgs->endianness);
                fprintf(stdout, "xor dword ptr [%s], 0x%x\n", pArgs->x64 ? "rsp" : "esp", otemp);
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
        char *string = pArgs->args[i]._str;
        int length = strlen(string);
        unsigned int sVal = 0;

        switch (pArgs->output) {
        case OMODE_RAW: {
            for (int i=length-1;i!=length-1-(length%4);i--) {
                sVal += string[i]<<(3*8-((i%4)*8));
            }
            if (sVal) {
                fputc('\x68', stdout);
                int xor = gen_xor(sVal);
                sVal ^= xor;
                fwrite(&xor, sizeof(int), 1, stdout);
                fputs("\x81\x34\x24", stdout);
                fwrite(&sVal, sizeof(int), 1, stdout);
            }
            sVal = 0;
            for (int i=length-1-(length%4);i!=-1;i--) {
                sVal += string[i]<<(3*8-((i%4)*8));

                if (i%4 == 0) {
                    fputc('\x68', stdout);
                    fwrite(&sVal, sizeof(int), 1, stdout);
                    sVal = 0;
                }
            }
            break;}
        case OMODE_HEXESC: {
            for (int i=length-1;i!=length-1-(length%4);i--) {
                sVal += string[i]<<(3*8-((i%4)*8));
            }
            if (sVal) {
                fputs("\\x68", stdout);
                int xor = gen_xor(sVal);
                sVal ^= xor;

                for (int i=0;i<4;i++) {
                    unsigned int hEsc = pad_hexc(((char*)&xor)[i]);
                    fprintf(stdout, "\\x%s", (char*)&hEsc);
                }
                fputs("\\x81\\x34\\x24", stdout);
                for (int i=0;i<4;i++) {
                    unsigned int hEsc = pad_hexc(((char*)&sVal)[i]);
                    fprintf(stdout, "\\x%s", (char*)&hEsc);
                }
            }
            sVal = 0;
            for (int i=length-1-(length%4);i!=-1;i--) {
                sVal += string[i]<<(3*8-((i%4)*8));

                if (i%4 == 0) {
                    fputs("\\x68", stdout);
                    for (int j=0;j<4;j++) {
                        unsigned int hEsc = pad_hexc(((char*)&sVal)[j]);
                        fprintf(stdout, "\\x%s", (char*)&hEsc);
                    }
                    sVal = 0;
                }
            }
            break;}
        case OMODE_ASM: {
            for (int i=length-1;i!=length-1-(length%4);i--) {
                sVal += string[i]<<(3*8-((i%4)*8));
            }
            if (sVal) {
                fputs("push 0x", stdout);
                int xor = gen_xor(sVal);
                sVal ^= xor;

                for (int i=0;i<4;i++) {
                    unsigned int hEsc = pad_hexc(((char*)&xor)[i]);
                    fputs((char*)&hEsc, stdout);
                }
                fprintf(stdout, "\nxor dword ptr [%s], 0x", pArgs->x64 ? "rsp" : "esp");

                for (int i=0;i<4;i++) {
                    unsigned int hEsc = pad_hexc(((char*)&sVal)[i]);
                    fputs((char*)&hEsc, stdout);
                }
            }
            sVal = 0;
            for (int i=length-1-(length%4);i!=-1;i--) {
                sVal += string[i]<<(3*8-((i%4)*8));

                if (i%4 == 0) {
                    fputs("\npush 0x", stdout);
                    for (int j=0;j<4;j++) {
                        unsigned int hEsc = pad_hexc(((char*)&sVal)[j]);
                        fputs((char*)&hEsc, stdout);
                    }
                    sVal = 0;
                }
            }
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
    case TYPE_STRING: {
        switch (pArgs->output) {
        case OMODE_RAW: {
            fputs(lRegisters[pArgs->x64][i].movs, stdout);
            break;}
        case OMODE_HEXESC: {
            char *movs = lRegisters[pArgs->x64][i].movs;
            for (int c=0;c<strlen(movs);c++) {
                unsigned int hEsc = pad_hexc(movs[c]);
                fprintf(stdout, "\\x%s", (char*)&hEsc);
            }
            break;}
        case OMODE_ASM: {
            fprintf(stdout, "\nmov %s, %s\n", lRegisters[pArgs->x64][i].name, pArgs->x64 ? "rsp" : "esp");
            break;}
        }
        break;}
    }
}