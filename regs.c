#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256
#define INSTR_BITS 16
#define DATA_SIZE 256
#define REG_COUNT 8
#define END_OPCODE 255
#define OVERFLOW_FLAG 1
#define BEQ_FLAG 0
#define NO_FLAG -1

struct IR{      //Struct para o Registrador de Instruções (RI)
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
};

struct REGS{
    struct IR RI;       //(RI)
    int RDM;            //Registrador de Dados da memória
    int A;              //Reg A (saída do banco de reg)
    int B;              //Reg A (saída do banco de reg)
    int ULASaida;
};
