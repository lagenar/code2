#include <iostream>
#include <cassert>

#include "code2.h"


int32_t sub (int32_t n, short izq, short der)
{
    n >>= der;
    return n & ((unsigned) 0xFFFFFFFF >> 31 - (izq - der));
}

void Code2::ejecutar_instruccion ()
{
    fetching ();
    char cnds[] = { 'R', 'Z', 'S', 'C', 'V' };
    uint8_t opcode = sub (RI, 15, 12);
    switch (opcode) {
    case 0x0:			//LD
        registros[sub (RI, 11, 8)] = mem[registros[0xD] + sub (RI, 7, 0)];
        printf ("LD R%X,%X\n", sub (RI, 11, 8), sub (RI, 7, 0));
        break;
    case 0x1:			//ST
        mem[registros[0xD] + sub (RI, 7, 0)] = registros[sub (RI, 11, 8)];
        printf ("ST %X,R%X\n", sub (RI, 7, 0), sub (RI, 11, 8));
        break;
    case 0x2:			//LLO
        registros[sub (RI, 11, 8)] = sub (RI, 7, 0);
        printf ("LLO R%X,%X\n", sub (RI, 11, 8), sub (RI, 7, 0));
        break;
    case 0x3:			//LHI
        registros[sub (RI, 11, 8)] =
            (sub (RI, 7, 0) << 8) + sub (registros[sub (RI, 11, 8)], 7, 0);
        printf ("LHI R%X,%X\n", sub (RI, 11, 8), sub (RI, 7, 0));
        break;
    case 0x4:			//IN
        break;
    case 0x5:			//OUT
        break;
    case 0x6: {		//ADDS
        int16_t a = registros[sub (RI, 7, 4)];
        int16_t b = registros[sub (RI, 3, 0)];
        int16_t r = a + b;
        registros[sub (RI, 11, 8)] = r;
        flags[Z] = (r == 0);
        flags[S] = r < 0;
        flags[V] = ((flags[S] && a > 0 && b > 0) || (!flags[S] && a < 0 && b < 0));
        flags[C] = ((uint32_t) a + (uint32_t) b > 0xFFFF);
        printf ("ADDS R%X,R%X,R%X\n", sub (RI, 11, 8), sub (RI, 7, 4),
                sub (RI, 3, 0));
        break;
    }
    case 0x7: {		//SUBS
        int16_t a = registros[sub (RI, 7, 4)];
        int16_t b = registros[sub (RI, 3, 0)];
        int16_t r = a - b;
        registros[sub (RI, 11, 8)] = r;
        flags[Z] = (r == 0);
        flags[S] = r < 0;
        flags[V] = ((!flags[S] && a < 0 && b > 0) || (flags[S] && a > 0 && b < 0));
        flags[C] = ((uint32_t) a - (uint32_t) b > 0xFFFF);	//REV
        printf ("SUBS R%X,R%X,R%X\n", sub (RI, 11, 8), sub (RI, 7, 4),
                sub (RI, 3, 0));
        break;
    }
    case 0x8: {		//NAND
        int16_t a = registros[sub (RI, 7, 4)];
        int16_t b = registros[sub (RI, 3, 0)];
        int16_t r = ~(a & b);
        registros[sub (RI, 11, 8)] = r;
        flags[Z] = (r == 0);
        flags[S] = (r < 0);
        printf ("NAND R%X,R%X,R%X\n", sub (RI, 11, 8), sub (RI, 7, 4),
                sub (RI, 3, 0));
        break;
    }
    case 0x9: {		//SHL
        int16_t r = registros[sub (RI, 11, 8)];
        flags[C] = sub (r, 15, 15);
        r <<= 1;
        flags[S] = sub (r, 15, 15) == 1;
        registros[sub (RI, 11, 8)] = r;
        printf ("SHL R%X\n", sub (RI, 11, 8));
        break;
    }
    case 0xA: {		//SHR
        int16_t r = registros[sub (RI, 11, 8)];
        flags[C] = sub (r, 0, 0);
        flags[S] = false;
        r = (uint16_t) r >> 1;
        flags[Z] = (r == 0);
        registros[sub (RI, 11, 8)] = r;
        printf ("SHR R%X\n", sub (RI, 11, 8));
        break;
    }
    case 0xB: {		//SHRA
        int16_t r = registros[sub (RI, 11, 8)];
        flags[C] = sub (r, 0, 0);
        r >>= 1;
        flags[Z] = (r == 0);
        registros[sub (RI, 11, 8)] = r;
        printf ("SHRA R%X\n", sub (RI, 11, 8));
        break;
    }
    case 0xC: {		//B(Branch)
        int16_t cnd = sub (RI, 11, 8);
        if ((cnd == 0x0) || (cnd == 0x1 && flags[Z]) || (cnd == 0x2 && flags[S]) ||
                (cnd == 0x3 && flags[C]) || (cnd == 0x4 && flags[V]))
            PC = registros[0xD];
        printf ("B%c\n", cnds[cnd]);
        break;
    }
    case 0xD: {		//CALL
        int16_t cnd = sub (RI, 11, 8);
        if ((cnd == 0x0) || (cnd == 0x1 && flags[Z]) || (cnd == 0x2 && flags[S]) ||
                (cnd == 0x3 && flags[C]) || (cnd == 0x4 && flags[V])) {
            mem[--registros[0xE]] = PC;
            PC = registros[0xD];
        }
        printf ("CALL%c\n", cnds[cnd]);
        break;
    }
    case 0xE:			// RET
        PC = mem[registros[0xE]];
        registros[0xE] -= 1;
        printf ("RET\n");
        break;
    case 0xF:			//HALT
        halt = true;
        printf ("HALT\n");
        break;
    default:
        break;
    }
}

int
main (int argc, char *argv[])
{
    return 0;
}
