#ifndef PARSER_H
#define PARSER_H

#include "stdint.h"
#include <iostream>

typedef enum {LD, ST, LLO, LHI, IN, OUT,
              ADDS, SUBS, NAND, SHL, SHR,
              SHRA, B, CALL, RET, HALT, NINGUNA
             } instruccion;

const uint16_t INSTR_INVALIDA = 0xFFFF;

uint16_t parsear_instruccion(const std::string &);

#endif
