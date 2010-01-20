#ifndef CODE2_H
#define CODE2_H
#include "stdint.h"
#include <iostream>
#include <cassert>

typedef enum {Z, S, C, V} Flag;

class Code2
{
    int16_t *mem;			//Buffer de memoria
    bool flags[4];
    int16_t registros[16];
    uint16_t PC;			//contador de programa
    int16_t RI;			//registro de instruccion
    bool halt;			//termino la ejecucion?

public:
    Code2(uint16_t inic) {
        PC = inic;
        halt = false;
        mem = new int16_t[65536];
    }

    ~Code2() {
        delete[]mem;
    }

    void set_mem(uint16_t pos, int16_t val) {
        mem[pos] = val;
    }

    int16_t get_mem(uint16_t pos) {
        return mem[pos];
    }

    bool termino_ejecucion() {
        return halt;
    }

    int16_t get_reg(int r) {
        return registros[r];
    }

    bool get_flag(Flag f) {
        return flags[f];
    }
    
    void print_flags() {
	 std::cout << "flags: {Z = " << (flags[Z] ? 1:0)
		   << "; C = " << (flags[C] ? 1:0)
		   << "; S = " << (flags[S] ? 1:0)
		   << "; V = " << (flags[V] ? 1:0) << "}" << std::endl;
    }

    void ejecutar_instruccion();

private:
    void fetching() {
        RI = mem[PC];
        PC++;
    }
};

#endif
