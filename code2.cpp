#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include "code2.h"
#include "parser.h"

#define UINT16_MAX 65535
#define INT16_MAX 32767

using namespace std;
//n[izq..der]
int32_t sub(int32_t n, short izq, short der)
{
    n >>= der;
    return n & ((unsigned) 0xFFFFFFFF >> 31 - (izq - der));
}

void Code2::ejecutar_instruccion()
{
     fetching();
     char cnds[] = { 'R', 'Z', 'S', 'C', 'V' };
     uint8_t opcode = sub(RI, 15, 12);
     switch (opcode) {
     case LD:
	  registros[sub(RI, 11, 8)] = mem[registros[0xD] + sub(RI, 7, 0)];
	  printf("LD R%X,%X\n", sub(RI, 11, 8), sub(RI, 7, 0));
	  break;
     case ST:
	  mem[registros[0xD] + sub(RI, 7, 0)] = registros[sub(RI, 11, 8)];
	  printf("ST %X,R%X\n", sub(RI, 7, 0), sub(RI, 11, 8));
	  break;
     case LLO:
	  registros[sub(RI, 11, 8)] = sub(RI, 7, 0);
	  printf("LLO R%X,%X\n", sub(RI, 11, 8), sub(RI, 7, 0));
	  break;
     case LHI:
	  registros[sub(RI, 11, 8)] =
	       (sub(RI, 7, 0) << 8) + sub(registros[sub(RI, 11, 8)], 7, 0);
	  printf("LHI R%X,%X\n", sub(RI, 11, 8), sub(RI, 7, 0));
	  break;
     case IN:
	  printf("IN\n");
	  break;
     case OUT:
	  printf("OUT\n");
	  break;
     case ADDS: {
	  int16_t a = registros[sub(RI, 7, 4)];
	  int16_t b = registros[sub(RI, 3, 0)];
	  int16_t r = a + b;
	  registros[sub(RI, 11, 8)] = r;
	  flags[Z] = (r == 0);
	  flags[S] = r < 0;
	  flags[V] = ((flags[S] && a > 0 && b > 0) || (!flags[S] && a < 0 && b < 0));
	  flags[C] = ((uint32_t) a + (uint32_t) b > 0xFFFF);
	  printf("ADDS R%X,R%X,R%X\n", sub(RI, 11, 8), sub(RI, 7, 4),
		 sub(RI, 3, 0));
	  break;
     }
     case SUBS: {
	  int16_t a = registros[sub(RI, 7, 4)];
	  int16_t b = registros[sub(RI, 3, 0)];
	  int16_t r = a - b;
	  registros[sub(RI, 11, 8)] = r;
	  flags[Z] = (r == 0);
	  flags[S] = r < 0;
	  flags[V] = ((!flags[S] && a < 0 && b > 0) || (flags[S] && a > 0 && b < 0));
	  flags[C] = ((uint32_t) a - (uint32_t) b > 0xFFFF);	//REV
	  printf("SUBS R%X,R%X,R%X\n", sub(RI, 11, 8), sub(RI, 7, 4),
		 sub(RI, 3, 0));
	  break;
     }
     case NAND: {
	  int16_t a = registros[sub(RI, 7, 4)];
	  int16_t b = registros[sub(RI, 3, 0)];
	  int16_t r = ~(a & b);
	  registros[sub(RI, 11, 8)] = r;
	  flags[Z] = (r == 0);
	  flags[S] = (r < 0);
	  printf("NAND R%X,R%X,R%X\n", sub(RI, 11, 8), sub(RI, 7, 4),
		 sub(RI, 3, 0));
	  break;
     }
     case SHL: {	
	  int16_t r = registros[sub(RI, 11, 8)];
	  flags[C] = sub(r, 15, 15);
	  r <<= 1;
	  flags[S] = sub(r, 15, 15) == 1;
	  registros[sub(RI, 11, 8)] = r;
	  printf("SHL R%X\n", sub(RI, 11, 8));
	  break;
     }
     case SHR: {
	  int16_t r = registros[sub(RI, 11, 8)];
	  flags[C] = sub(r, 0, 0);
	  flags[S] = false;
	  r = (uint16_t) r >> 1;
	  flags[Z] = (r == 0);
	  registros[sub(RI, 11, 8)] = r;
	  printf("SHR R%X\n", sub(RI, 11, 8));
	  break;
     }
     case SHRA: {
	  int16_t r = registros[sub(RI, 11, 8)];
	  flags[C] = sub(r, 0, 0);
	  r >>= 1;
	  flags[Z] = (r == 0);
	  registros[sub(RI, 11, 8)] = r;
	  printf("SHRA R%X\n", sub(RI, 11, 8));
	  break;
     }
     case B: {
	  int16_t cnd = sub(RI, 11, 8);
	  if ((cnd == 0x0) || (cnd == 0x1 && flags[Z]) || (cnd == 0x2 && flags[S]) ||
	      (cnd == 0x3 && flags[C]) || (cnd == 0x4 && flags[V]))
	       PC = registros[0xD];
	  printf("B%c\n", cnds[cnd]);
	  break;
     }
     case CALL: {
	  int16_t cnd = sub(RI, 11, 8);
	  if ((cnd == 0x0) || (cnd == 0x1 && flags[Z]) || (cnd == 0x2 && flags[S]) ||
	      (cnd == 0x3 && flags[C]) || (cnd == 0x4 && flags[V])) {
	       mem[--registros[0xE]] = PC;
	       PC = registros[0xD];
	  }
	  printf("CALL%c\n", cnds[cnd]);
	  break;
     }
     case RET:
	  PC = mem[registros[0xE]];
	  registros[0xE] -= 1;
	  printf("RET\n");
	  break;
     case HALT:
	  halt = true;
	  printf("HALT\n");
	  break;
     default:
	  break;
     }
}

void uso(const char* argv0)
{
     cout << "Uso: " <<  argv0 << " [-OPCION] [FICHERO]" << endl << endl;
     cout << "\t-o\t Para especificar la direccion de carga del programa."
	  "\n\t \t Por defecto es 0x100." << endl;
}

int main(int argc, char *argv[])
{
     uint16_t offset = 0x100;
     int c;     
     while ((c = getopt(argc, argv, "o:")) != -1) {
	  long off;
	  switch (c) {
	  case 'o': {
	       off = strtol(optarg, NULL, 16);
	       if (off < 0 || off > UINT16_MAX) {
		    cout << "offset fuera de rango" << endl;
		    exit(1);
	       } else
		    offset = (uint16_t)off;
	       break;
	  }
	  default:
	       break;
	  }
     }
     if (optind >= argc) {
	  uso(argv[0]);
	  exit(1);
     }
     ifstream arch(argv[optind]);
     if (!arch.is_open()) {
	  cout << "No se pudo abrir el archivo" << endl;
	  exit(1);
     }
     Code2 code2(offset);
     string s_instr;
     int linea = 1;
     cout << "Cargando programa... ";
     while (getline(arch, s_instr)) {
	  uint16_t instr = parsear_instruccion(s_instr);
	  if (instr == INSTR_INVALIDA) {
	       cout << "Instruccion invalida en la linea " << linea
			 << ": " << s_instr << endl;
	       exit(1);
	  }
	  code2.set_mem(offset, (int16_t)instr);
	  ++offset;
	  ++linea;
     }
     cout << "Listo." << endl << endl;
     cout << "Ingrese h para ver los comandos" << endl;
     bool halt = false;
     bool ejecutar_todo = false;
     while (!halt) {
	  if (!ejecutar_todo) {
	       string comando;
	       getline(cin, comando);
	       istringstream iss(comando);
	       vector<string> tokens;
	       copy(istream_iterator<string>(iss),
		    istream_iterator<string>(),
		    back_inserter<vector<string> >(tokens));
	       if (tokens.size() > 0) {
		    if (tokens[0] == "n")
			 code2.ejecutar_instruccion();
		    else if (tokens[0] == "f")
			 code2.print_flags();
	       }
	  } else {
	       code2.ejecutar_instruccion();
	  }
	  halt = halt || code2.termino_ejecucion();
     }	  
     return 0;
}
