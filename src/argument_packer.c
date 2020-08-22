#include <shellgen/argument_packer.h>

typedef unsigned long long ull;

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
    if (endianness == ENDIANNESS_BIG) {
        return (((char*)&target)[0]<<24) | (((char*)&target)[1]<<16) | (((char*)&target)[2]<<8) | (((char*)&target)[3]<<0);
    } else {
        return (((char*)&target)[3]<<24) | (((char*)&target)[2]<<16) | (((char*)&target)[1]<<8) | (((char*)&target)[0]<<0);
    }
}

unsigned long long set_endianness_64(unsigned long long target, char endianness)
{
    if (endianness == ENDIANNESS_BIG) {
        return ((ull)((char*)&target)[0]<<56) | ((ull)((char*)&target)[1]<<48) | ((ull)((char*)&target)[2]<<40) | ((ull)((char*)&target)[3]<<32) | \
            ((ull)((char*)&target)[4]<<24) | ((ull)((char*)&target)[5]<<16) | ((ull)((char*)&target)[6]<<8) | ((ull)((char*)&target)[7]<<0);
    } else {
        return ((ull)((char*)&target)[7]<<56) | ((ull)((char*)&target)[6]<<48) | ((ull)((char*)&target)[5]<<40) | ((ull)((char*)&target)[4]<<32) | \
            ((ull)((char*)&target)[3]<<24) | ((ull)((char*)&target)[2]<<16) | ((ull)((char*)&target)[1]<<8) | ((ull)((char*)&target)[0]<<0);
    }
}


// check nullbytes
bool has_nullbytes(int n)
{
    for (int i=0;i<4;i++) {
        if (((char*)&n)[i] == 0) {
            return true;
        }
    }
    return false;
}

bool has_nullbytes_64(unsigned long long n)
{
    for (int i=0;i<8;i++) {
        if (((char*)&n)[i] == 0) {
            return true;
        }
    }
    return false;
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

unsigned long long gen_xor_64(unsigned long long n)
{
    unsigned long long result = 0;
    for (int i=0;i<8;i++) {
        unsigned long long c = (n>>(i*8)) & 0xFF;
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
        unsigned long long sVal_64 = 0;
        struct sRegister *curr_reg = &lRegisters[pArgs->x64][i];

        if (pArgs->x64) {
            /*
                    XORing like:
                    mov [reg], xor1
                    mov r11, xor2
                    xor [reg], r11
                */

            switch (pArgs->output) {
            case OMODE_RAW: {
                for (int i=length-1;i!=length-1-(length%8);i--) {
                    unsigned long long buf = string[i];
                    sVal_64 += buf << 8 * (length%8 - (length - i));
                }

                if (sVal_64) {
                    unsigned long long xor = gen_xor_64(sVal_64);
                    sVal_64 ^= xor;

                    xor = set_endianness_64(xor, ENDIANNESS_LITTLE);
                    sVal_64 = set_endianness_64(sVal_64, ENDIANNESS_LITTLE);

                    fputs(curr_reg->mov, stdout);
                    fwrite(&xor, sizeof(unsigned long long), 1, stdout);
                    fputs(r11.mov, stdout);
                    fwrite(&sVal_64, sizeof(unsigned long long), 1, stdout);
                    fputs(curr_reg->xor11, stdout);
                    fputs(curr_reg->push, stdout);
                }

                sVal_64 = 0;
                for (int i=length-1-(length%8);i!=-1;i--) {
                    unsigned long long buf = string[i];
                    sVal_64 += buf<<((i%8)*8);

                    if (i%8 == 0) {
                        fputs(curr_reg->mov, stdout);
                        fwrite(&sVal_64, sizeof(unsigned long long), 1, stdout);
                        fputs(curr_reg->push, stdout);
                        sVal_64 = 0;
                    }
                }
                break;}
            case OMODE_HEXESC: {
                for (int i=length-1;i!=length-1-(length%8);i--) {
                    unsigned long long buf = string[i];
                    sVal_64 += buf << 8 * (length%8 - (length - i));
                }

                if (sVal_64) {
                    unsigned long long xor = gen_xor_64(sVal_64);
                    sVal_64 ^= xor;

                    xor = set_endianness_64(xor, ENDIANNESS_LITTLE);
                    sVal_64 = set_endianness_64(sVal_64, ENDIANNESS_LITTLE);

                    for (int i=0;i<strlen(curr_reg->mov);i++) {
                        unsigned int hEsc = pad_hexc(curr_reg->mov[i]);
                        fprintf(stdout, "\\x%s", (char*)&hEsc);
                    } // mov [reg], 

                    for (int i=0;i<8;i++) {
                        unsigned int hEsc = pad_hexc(((char*)&xor)[i]);
                        fprintf(stdout, "\\x%s", (char*)&hEsc);
                    } // xor1

                    for (int i=0;i<strlen(r11.mov);i++) {
                        unsigned int hEsc = pad_hexc(r11.mov[i]);
                        fprintf(stdout, "\\x%s", (char*)&hEsc);
                    } //  mov r11, 

                    for (int i=0;i<8;i++) {
                        unsigned int hEsc = pad_hexc(((char*)&sVal_64)[i]);
                        fprintf(stdout, "\\x%s", (char*)&hEsc);
                    } // xor2

                    for (int i=0;i<strlen(curr_reg->xor11);i++) {
                        unsigned int hEsc = pad_hexc(curr_reg->xor11[i]);
                        fprintf(stdout, "\\x%s", (char*)&hEsc);
                    } // xor [reg], r11

                    for (int i=0;i<strlen(curr_reg->push);i++) {
                        unsigned int hEsc = pad_hexc(curr_reg->push[i]);
                        fprintf(stdout, "\\x%s", (char*)&hEsc);
                    } // push [reg]

                }
                sVal_64 = 0;
                for (int i=length-1-(length%8);i!=-1;i--) {
                    unsigned long long buf = string[i];
                    sVal_64 += buf<<((i%8)*8);

                    if (i%8 == 0) {
                        for (int j=0;j<strlen(curr_reg->mov);j++) {
                            unsigned int hEsc = pad_hexc(curr_reg->mov[j]);
                            fprintf(stdout, "\\x%s", (char*)&hEsc);
                        } // mov [reg], ...
                        for (int j=0;j<8;j++) {
                            unsigned int hEsc = pad_hexc(((char*)&sVal_64)[j]);
                            fprintf(stdout, "\\x%s", (char*)&hEsc);
                        }
                        for (int j=0;j<strlen(curr_reg->push);j++) {
                            unsigned int hEsc = pad_hexc(curr_reg->push[j]);
                            fprintf(stdout, "\\x%s", (char*)&hEsc);
                        } // push [reg]
                        sVal_64 = 0;
                    }
                }
                break;}
            case OMODE_ASM: {
                for (int i=length-1;i!=length-1-(length%8);i--) {
                    unsigned long long buf = string[i];
                    sVal_64 += buf << 8 * (length%8 - (length - i));
                }

                if (sVal_64) {
                    unsigned long long xor = gen_xor_64(sVal_64);
                    sVal_64 ^= xor;

                    xor = set_endianness_64(xor, ENDIANNESS_BIG);
                    sVal_64 = set_endianness_64(sVal_64, ENDIANNESS_BIG);

                    fprintf(stdout, "mov %s, 0x", curr_reg->name);
                    for (int i=0;i<8;i++) {
                        unsigned int hEsc = pad_hexc(((char*)&xor)[i]);
                        fputs((char*)&hEsc, stdout);
                    }
                    fputs("\nmov r11, 0x", stdout);
                    for (int i=0;i<8;i++) {
                        unsigned int hEsc = pad_hexc(((char*)&sVal_64)[i]);
                        fputs((char*)&hEsc, stdout);
                    }
                    fprintf(stdout, "\nxor %s, r11\npush %s\n", curr_reg->name, curr_reg->name);
                }
                sVal_64 = 0;
                for (int i=length-1-(length%8);i!=-1;i--) {
                    unsigned long long buf = string[i];
                    sVal_64 += buf<<((i%8)*8);

                    if (i%8 == 0) {
                        fprintf(stdout, "mov %s, 0x", curr_reg->name);
                        for (int j=0;j<8;j++) {
                            unsigned int hEsc = pad_hexc(((char*)&sVal_64)[7-j]);
                            fputs((char*)&hEsc, stdout);
                        }
                        fprintf(stdout, "\npush %s\n", curr_reg->name);
                        sVal_64 = 0;
                    }
                }
                break;}
            }
        } else {
            switch (pArgs->output) {
            case OMODE_RAW: {
                for (int i=length-1;i!=length-1-(length%4);i--) {
                    //sVal += string[i]<<(3*8-((i%4)*8));
                    sVal += string[i] << 8 * (length%4 - (length - i));
                }

                if (sVal) {
                    fputc('\x68', stdout);
                    int xor = gen_xor(sVal);
                    sVal ^= xor;

                    xor = set_endianness(xor, (pArgs->endianness==ENDIANNESS_LITTLE) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE);
                    sVal = set_endianness(sVal, (pArgs->endianness==ENDIANNESS_LITTLE) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE);

                    fwrite(&xor, sizeof(int), 1, stdout);
                    fputs("\x81\x34\x24", stdout);
                    fwrite(&sVal, sizeof(int), 1, stdout);
                }

                sVal = 0;
                for (int i=length-1-(length%4);i!=-1;i--) {
                    sVal += string[i]<<((i%4)*8);

                    if (i%4 == 0) {
                        fputc('\x68', stdout);
                        fwrite(&sVal, sizeof(int), 1, stdout);
                        sVal = 0;
                    }
                }
             break;}
            case OMODE_HEXESC: {
                for (int i=length-1;i!=length-1-(length%4);i--) {
                    sVal += string[i] << 8 * (length%4 - (length - i));
                }
                if (sVal) {
                    fputs("\\x68", stdout);
                    int xor = gen_xor(sVal);
                    sVal ^= xor;

                    xor = set_endianness(xor, (pArgs->endianness==ENDIANNESS_LITTLE) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE);
                    sVal = set_endianness(sVal, (pArgs->endianness==ENDIANNESS_LITTLE) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE);

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
                    sVal += string[i]<<((i%4)*8);

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
                    sVal += string[i] << 8 * (length%4 - (length - i));
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
                    sVal += string[i]<<(3*8-(i%4)*8);

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
        }} break;}
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