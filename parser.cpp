#include "parser.h"
#include <cctype>
#include <cassert>
#include <cstdlib>

const std::string repr[] =  {"LD", "ST", "LLO", "LHI", "IN", "OUT",
                             "ADDS", "SUBS", "NAND", "SHL", "SHR",
                             "SHRA", "B", "CALL", "RET", "HALT"
                            };

size_t saltear_espacios(const std::string & str, size_t pos = 0)
{
    if (pos >= str.size() || pos == std::string::npos)
        return std::string::npos;
    while (pos < str.size() && isspace(str[pos]))
        pos++;

    if (pos == str.size())
        return std::string::npos;
    return pos;
}

instruccion leer_instruccion(const std::string & str, std::string & instr, size_t & pos)
{
    pos = std::string::npos;

    size_t pos_inic = saltear_espacios(str);
    if (pos_inic == std::string::npos)
        return NINGUNA;

    size_t pos_fin = str.find_first_of(" \t", pos_inic);
    if (pos_fin == std::string::npos)
        instr = str.substr(pos_inic, str.size() - pos_inic + 1);
    else
        instr = str.substr(pos_inic, pos_fin - pos_inic);

    //CALL y B se manejan en forma separada porque el campo cnd es parte
    //del nombre de la instruccion

    if ((instr.size() == 2 && instr[0] == 'B') ||
            (instr.size() == 5 && instr.substr(0, 4) == "CALL"))
        switch (instr[instr.size() - 1]) {
        case 'R':
        case 'V':
        case 'Z':
        case 'C':
        case 'S':
            pos = pos_fin;
            return instr[0] == 'B' ? B : CALL;
            break;
        default:
            return NINGUNA;
            break;
        }

    bool encontrada = false;
    int i = LD;
    while (!encontrada && i <= HALT) {
        if (i != B && i != CALL) {
            if (instr == repr[i])
                encontrada = true;
        }
        if (!encontrada)
            i++;
    }

    if (encontrada) {
        pos = pos_fin;
        return (instruccion)i;
    }
    return NINGUNA;
}

int leer_registro(const std::string & str, size_t & pos)
{
    pos = saltear_espacios(str, pos);
    if (pos == std::string::npos ||
            str.size() - pos < 2 ||
            str[pos] != 'R' ||
            !isxdigit(str[pos+1])) {
        pos = std::string::npos;
        return -1;
    }

    int reg = strtol(str.substr(pos + 1, 1).c_str(), NULL, 16);
    pos += 2;
    if (pos >= str.size())
        pos = std::string::npos;

    return reg;
}

//lee el campo v(de 8 bits) para las instrucciones de tipo 3
int leer_campo_v(const std::string & str, size_t & pos)
{
    pos = saltear_espacios(str, pos);
    if (pos == std::string::npos ||
            str.size() - pos < 2 ||
            !isxdigit(str[pos]) || !isxdigit(str[pos + 1])) {
        pos = std::string::npos;
        return -1;
    }

    int v = strtol(str.substr(pos, 2).c_str(), NULL, 16);
    pos += 2;
    if (pos >= str.size())
        pos = std::string::npos;

    return v;
}


uint16_t parsear_instruccion_tipo0(instruccion i, const std::string & str, size_t pos)
{
    if (saltear_espacios(str, pos) != std::string::npos)
        return INSTR_INVALIDA;
    return (uint16_t)i << 12;
}

uint16_t parsear_instruccion_tipo1(instruccion i, const std::string & str, size_t pos)
{
    int reg = leer_registro(str, pos);
    if (reg == -1 ||
            (saltear_espacios(str, pos) != std::string::npos))
        return INSTR_INVALIDA;

    return (uint16_t)i << 12 | (reg << 8);
}

uint16_t parsear_instruccion_tipo2(instruccion i, char cnd, const std::string & str,
                                   size_t pos)
{
    if (saltear_espacios(str, pos) != std::string::npos)
        return INSTR_INVALIDA;

    uint16_t op = (uint16_t)i << 12;

    switch (cnd) {
    case 'R':
        break;
    case 'Z':
        op |= 0x0100;
        break;
    case 'S':
        op |= 0x0200;
        break;
    case 'C':
        op |= 0x0300;
        break;
    case 'V':
        op |= 0x0400;
        break;
    default:
        op = INSTR_INVALIDA;
        break;
    }
    return op;
}

uint16_t parsear_instruccion_tipo3(instruccion i, const std::string & str, size_t pos)
{
    int v;
    int reg;
    switch (i) {
    case LD:
    case LLO:
    case LHI:
    case IN:
        reg = leer_registro(str, pos);
        if (reg == -1 || pos == std::string::npos)
            return INSTR_INVALIDA;
        else {
            pos = saltear_espacios(str, pos);
            if (pos == std::string::npos || str[pos] != ',')
                return INSTR_INVALIDA;
            else {
                pos++;
                v = leer_campo_v(str, pos);
                if (v == -1 ||
                        (saltear_espacios(str, pos) != std::string::npos))
                    return INSTR_INVALIDA;
            }
        }
        break;
    case OUT:
    case ST:
        v = leer_campo_v(str, pos);
        if (v == -1 || pos == std::string::npos)
            return INSTR_INVALIDA;
        else {
            pos = saltear_espacios(str, pos);
            if (pos == std::string::npos || str[pos] != ',')
                return INSTR_INVALIDA;
            else {
                pos++;
                reg = leer_registro(str, pos);
                if (reg == -1 ||
                        (saltear_espacios(str, pos) != std::string::npos))
                    return INSTR_INVALIDA;
            }
        }
        break;
    default:
        break;
    }

    return (uint16_t)i << 12 | reg << 8 | v;
}

uint16_t parsear_instruccion_tipo4(instruccion i, const std::string & str, size_t pos)
{
    int r1 = leer_registro(str, pos);

    if (r1 == -1)
        return INSTR_INVALIDA;
    pos = saltear_espacios(str, pos);
    if (pos == std::string::npos || str[pos] != ',')
        return INSTR_INVALIDA;

    pos++;
    int r2 = leer_registro(str, pos);
    if (r2 == -1)
        return INSTR_INVALIDA;
    pos = saltear_espacios(str, pos);
    if (pos == std::string::npos || str[pos] != ',')
        return INSTR_INVALIDA;

    pos++;
    int r3 = leer_registro(str, pos);
    if (r3 == -1 || saltear_espacios(str, pos) != std::string::npos)
        return INSTR_INVALIDA;

    return (uint16_t)i << 12 | r1 << 8 | r2 << 4 | r3;
}

uint16_t parsear_instruccion(const std::string & ins)
{
    size_t pos = 0;
    std::string nom_inst;
    instruccion i = leer_instruccion(ins, nom_inst, pos);

    switch (i) {
    case RET:
    case HALT:
        return parsear_instruccion_tipo0(i, ins, pos);
        break;
    case SHR:
    case SHRA:
    case SHL:
        return parsear_instruccion_tipo1(i, ins, pos);
        break;
    case B:
    case CALL:
        return parsear_instruccion_tipo2(i, nom_inst[nom_inst.size() - 1], ins, pos);
        break;
    case LD:
    case ST:
    case LLO:
    case LHI:
    case IN:
    case OUT:
        return parsear_instruccion_tipo3(i, ins, pos);
        break;
    case ADDS:
    case SUBS:
    case NAND:
        return parsear_instruccion_tipo4(i, ins, pos);
        break;
    default:
        return INSTR_INVALIDA;
        break;
    }
}

