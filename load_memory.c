#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256
#define INSTR_BITS 16
#define DATA_START 128

enum classe_inst { tipo_R, tipo_I, tipo_J, tipo_dado, tipo_INVALIDO };

struct inst_dados {
    enum classe_inst tipo;
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
    int dado;
    char binario[INSTR_BITS + 1];
};

typedef struct {
    struct inst_dados instr_decod[MEM_SIZE]; // Memória decodificada
    int num_instrucoes;
} Memory;

void load_memory(Memory *memory, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    char line[INSTR_BITS + 2];
    int i = 0;                 // Índice para instruções
    int data_mode = 0;         // 0 = instruções, 1 = dados
    int data_index = DATA_START;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, ".data") == 0) {
            data_mode = 1;
            continue;
        }

        if (strlen(line) == 0) continue;

        // Preenche com zeros à esquerda
        if (strlen(line) < INSTR_BITS) {
            int zeros = INSTR_BITS - strlen(line);
            char temp[INSTR_BITS + 1] = {0};
            memset(temp, '0', zeros);
            strcat(temp, line);
            strcpy(line, temp);
        }

        if (strlen(line) != INSTR_BITS) continue;

        if (!data_mode) {
            // Processa instrução
            if (i >= DATA_START) {
                printf("Erro: Limite de instruções excedido\n");
                break;
            }
            
            strncpy(memory->instr_decod[i].binario, line, INSTR_BITS);
			memory->instr_decod[i].binario[INSTR_BITS] = '\0'; // Terminador nulo

            memory->instr_decod[i].opcode = strtol(line, NULL, 2) >> 12; // Pega os 4 primeiros bits

            if (memory->instr_decod[i].opcode == 0) {
                // Tipo R
                memory->instr_decod[i].tipo = tipo_R;
                memory->instr_decod[i].rs = (strtol(line, NULL, 2) >> 9) & 0x7;  // bits 4-6
                memory->instr_decod[i].rt = (strtol(line, NULL, 2) >> 6) & 0x7;  // bits 7-9
                memory->instr_decod[i].rd = (strtol(line, NULL, 2) >> 3) & 0x7;  // bits 10-12
                memory->instr_decod[i].funct = strtol(line, NULL, 2) & 0x7;      // bits 13-15
            } 
            else if (memory->instr_decod[i].opcode == 2) {
                // Tipo J
                memory->instr_decod[i].tipo = tipo_J;
                memory->instr_decod[i].addr = strtol(line, NULL, 2) & 0xFFF;     // bits 4-15
            } 
            else {
                // Tipo I
                memory->instr_decod[i].tipo = tipo_I;
                memory->instr_decod[i].rs = (strtol(line, NULL, 2) >> 9) & 0x7;  // bits 4-6
                memory->instr_decod[i].rt = (strtol(line, NULL, 2) >> 6) & 0x7;  // bits 7-9
                memory->instr_decod[i].imm = strtol(line, NULL, 2) & 0x3F;       // bits 10-15
            }
            i++;
        } else {
            // Processa dado
            if (data_index >= MEM_SIZE) {
                printf("Erro: Memória de dados cheia\n");
                break;
            }
            
            strncpy(memory->instr_decod[data_index].binario, line, INSTR_BITS);
			memory->instr_decod[data_index].binario[INSTR_BITS] = '\0';

            memory->instr_decod[data_index].tipo = tipo_dado;
            memory->instr_decod[data_index].dado = strtol(line, NULL, 2);
            data_index++;
        }
    }
    memory->num_instrucoes = i;
    fclose(file);
}

void print_memory(const Memory *memory) {
    printf("\n=== Memória Decodificada ===\n");
    printf("End. | Binário           | Tipo | Opcode | rs | rt | rd | funct | imm  | addr  | Dado\n");
    printf("-----------------------------------------------------------------------------------------\n");

    for (int i = 0; i < MEM_SIZE; i++) {
        // Mostra apenas posições com conteúdo válido
        if (i < memory->num_instrucoes || (i >= DATA_START && memory->instr_decod[i].tipo == tipo_dado)) {
            printf("%3d  | %-16s | ", i, memory->instr_decod[i].binario);
            
            switch(memory->instr_decod[i].tipo) {
                case tipo_R:
                    printf("R  | %6d | %2d | %2d | %2d | %5d |      |       |\n",
                           memory->instr_decod[i].opcode,
                           memory->instr_decod[i].rs,
                           memory->instr_decod[i].rt,
                           memory->instr_decod[i].rd,
                           memory->instr_decod[i].funct);
                    break;
                    
                case tipo_I:
                    printf("I  | %6d | %2d | %2d |    |       | %4d |       |\n",
                           memory->instr_decod[i].opcode,
                           memory->instr_decod[i].rs,
                           memory->instr_decod[i].rt,
                           memory->instr_decod[i].imm);
                    break;
                    
                case tipo_J:
                    printf("J  | %6d |    |    |    |       |      | %5d |\n",
                           memory->instr_decod[i].opcode,
                           memory->instr_decod[i].addr);
                    break;
                    
                case tipo_dado:
                    printf("DADO |       |    |    |    |       |      |       | %5d\n",
                           memory->instr_decod[i].dado);
                    break;
                    
                default:
                    printf("?  | %6d |    |    |    |       |      |       |\n",
                           memory->instr_decod[i].opcode);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_instrucoes.txt>\n", argv[0]);
        return 1;
    }

    Memory mem = {0};
    load_memory(&mem, argv[1]);
    print_memory(&mem);

    return 0;
}
