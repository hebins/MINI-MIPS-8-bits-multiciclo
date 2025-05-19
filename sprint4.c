#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ================= CONSTANTES E DEFINES =================
#define MEM_SIZE 256
#define INSTR_BITS 16
#define REG_COUNT 8
#define DATA_START 128
#define END_OPCODE 255
#define OVERFLOW_FLAG 1
#define BEQ_FLAG 0
#define NO_FLAG -1

// ================= ESTRUTURAS DE DADOS =================
enum classe_inst { tipo_R, tipo_I, tipo_J, tipo_dado, tipo_INVALIDO };

enum ops_ula { ULA_ADD, ULA_SUB, ULA_AND, ULA_OR };

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

struct pc {
    int valor;
    int prev_valor;
};

typedef struct regs {
    struct inst_dados RI;
    int RDM;
    int A;
    int B;
    int ULASaida;
} REGS;

struct estado_processador {
    Memory memory;
    int registradores[REG_COUNT];
    struct pc pc;
    int halt_flag;
    int passos_executados;
    REGS regs;
    int step_atual;
};

// ================= PROTÓTIPOS DE FUNÇÕES =================
void print_memory(const Memory *memory);
void load_memory(Memory *memory, const char *filename);
void executa_instrucao(struct estado_processador *estado);
int ula(enum ops_ula operacao, int a, int b, int *flag);
void mostrar_registradores(int registradores[]);
void print_instrucao(const struct inst_dados *ri);
void display_menu_principal();
void inicializar_processador(struct estado_processador *cpu);
void display_menu_execucao();
void step(struct estado_processador *estado);
int binario_para_decimal(const char *binario);
void armazenaRI(struct estado_processador *estado);
void criaasm(struct estado_processador *estado, const char *nome_arquivo);
void salvar_estado_para_arquivo(struct estado_processador *estado, const char *filename);
void mostrar_estado_processador(struct estado_processador *estado);

// ================= FUNÇÃO PRINCIPAL =================
int main(void) {
    struct estado_processador cpu;
    char filename[256];
    inicializar_processador(&cpu);
    
    int option;
    int em_execucao = 0;
    int sair = 0;

    do {
        if (em_execucao && !cpu.halt_flag) {
            display_menu_execucao();
        } else {
            display_menu_principal();
        }
        
        scanf("%d", &option);
        getchar();
        
        if (em_execucao && !cpu.halt_flag) {
            switch(option) {
                case 1: step(&cpu); break;
                case 2: mostrar_registradores(cpu.registradores); break;
                case 3: print_memory(&cpu.memory); break;
                case 4: break;
                case 5: break;
                case 6: break;
                case 7: break;
                case 8: em_execucao = 0; break;
                case 9: printf("Total de instruções executadas: %d\n", cpu.passos_executados); break;
                case 10: while (!cpu.halt_flag) step(&cpu); break;
                default: printf("Opção inválida!\n");
            }
        } else {
            switch(option) {
                case 1:
                    printf("Digite o nome do arquivo de instruções: ");
                    fgets(filename, sizeof(filename), stdin);
                    filename[strcspn(filename, "\n")] = '\0';
                    load_memory(&cpu.memory, filename);
                    break;
                case 2:
                    if (cpu.memory.num_instrucoes > 0) {
                        em_execucao = 1;
                        cpu.halt_flag = 0;
                        printf("Modo execução passo a passo ativado\n");
                    } else {
                        printf("Erro: Nenhuma instrução carregada\n");
                    }
                    break;
                case 3: 
                    mostrar_estado_processador(&cpu);
					break;
                case 4: 
					printf("Digite o nome do arquivo para salvar: ");
					char filename[256];
					fgets(filename, sizeof(filename), stdin);
					filename[strcspn(filename, "\n")] = '\0';
					salvar_estado_para_arquivo(&cpu, filename);
					break;
                case 5:
                if (cpu.memory.num_instrucoes > 0) {
                        printf("Digite o nome do arquivo de saída (.asm): ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = '\0';
                        criaasm(&cpu, filename);
                    } else {
                        printf("Erro: Nenhuma instrução carregada para converter\n");
                    }
                    break;
                case 6: sair = 1; break;
                default: printf("Opção inválida!\n");
            }
        }
    } while (!sair);

    return 0;
}

// ================= FUNÇÕES AUXILIARES =================

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

int ula(enum ops_ula operacao, int a, int b, int *flag) {
    if (flag) *flag = NO_FLAG;
    int resultado;
    
    switch (operacao) {
        case ULA_ADD:
            resultado = a + b;
            if ((a > 0 && b > 0 && resultado < 0) || (a < 0 && b < 0 && resultado > 0)) {
                if (flag) *flag = OVERFLOW_FLAG;
            }
            break;
        case ULA_SUB:
            resultado = a - b;
            if ((a > 0 && b < 0 && resultado < 0) || (a < 0 && b > 0 && resultado > 0)) {
                if (flag) *flag = OVERFLOW_FLAG;
            }
            if (resultado == 0 && flag) {
                *flag = BEQ_FLAG;
            }
            break;
        case ULA_AND:
            resultado = a & b;
            break;
        case ULA_OR:
            resultado = a | b;
            break;
        default:
            resultado = 0;
    }
    return resultado;
}

void mostrar_registradores(int registradores[]) {
    printf("\n=== Registradores ===\n");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("$%d = %d\n", i, registradores[i]);
    }
}

void print_instrucao(const struct inst_dados *ri) {
    printf("Binário: %s\n", ri->binario);
    printf("Opcode: %d | Tipo: ", ri->opcode);
    switch (ri->tipo) {
        case tipo_R: printf("R | rs: %d, rt: %d, rd: %d, funct: %d\n", ri->rs, ri->rt, ri->rd, ri->funct); break;
        case tipo_I: printf("I | rs: %d, rt: %d, imm: %d\n", ri->rs, ri->rt, ri->imm); break;
        case tipo_J: printf("J | addr: %d\n", ri->addr); break;
        default: printf("Inválido\n");
    }
}

void inicializar_processador(struct estado_processador *cpu) {
    memset(cpu, 0, sizeof(struct estado_processador));
    cpu->pc.valor = 0;
    cpu->pc.prev_valor = -1;
    cpu->halt_flag = 1;
    cpu->passos_executados = 0;
    cpu->step_atual = 0;
    
    for (int i = 0; i < REG_COUNT; i++) {
        cpu->registradores[i] = 0;
    }
}

void display_menu_principal() {
    printf("\n=== MENU PRINCIPAL ===\n");
    printf("1. Carregar programa\n");
    printf("2. Iniciar execução passo a passo\n");
    printf("3. Mostrar estado do processador\n");
    printf("4. Salvar estado do processador\n");
    printf("5. Gerar arquivo assembly\n"); 
    printf("6. Sair\n");
    printf("======================\n");
    printf("Escolha uma opção: ");
}

void display_menu_execucao() {
    printf("\n=== MENU DE EXECUÇÃO ===\n");
    printf("1. Executar próxima instrução\n");
    printf("2. Mostrar registradores\n");
    printf("3. Mostrar memória completa\n");
    //printf("4. Mostrar memória de dados\n");
    printf("5. Salvar instruções executadas\n");
    printf("6. Salvar memória de dados\n");
    printf("7. Voltar instrução anterior\n");
    printf("8. Voltar ao menu principal\n");
    printf("9. Mostrar total de instruções executadas\n");
    printf("10. Executar todas as instruções\n");
    printf("=========================\n");
    printf("Escolha uma opção: ");
}

void armazenaRI(struct estado_processador *estado){

    if(estado->memory.instr_decod[estado->pc.valor].opcode == 0){
        estado->memory.instr_decod[estado->pc.valor].tipo = estado->regs.RI.tipo;
        estado->memory.instr_decod[estado->pc.valor].opcode = estado->regs.RI.opcode;
        estado->memory.instr_decod[estado->pc.valor].rs = estado->regs.RI.rs;
        estado->memory.instr_decod[estado->pc.valor].rt = estado->regs.RI.rt;
        estado->memory.instr_decod[estado->pc.valor].rd = estado->regs.RI.rd;
        estado->memory.instr_decod[estado->pc.valor].funct = estado->regs.RI.funct;
    } else if(estado->memory.instr_decod[estado->pc.valor].opcode == 2){
        estado->memory.instr_decod[estado->pc.valor].tipo = estado->regs.RI.tipo;
        estado->memory.instr_decod[estado->pc.valor].opcode = estado->regs.RI.opcode;
        estado->memory.instr_decod[estado->pc.valor].addr = estado->regs.RI.addr;
    } else {
        estado->memory.instr_decod[estado->pc.valor].tipo = estado->regs.RI.tipo;
        estado->memory.instr_decod[estado->pc.valor].opcode = estado->regs.RI.opcode;
        estado->memory.instr_decod[estado->pc.valor].rs = estado->regs.RI.rs;
        estado->memory.instr_decod[estado->pc.valor].rt = estado->regs.RI.rt;
        estado->memory.instr_decod[estado->pc.valor].imm = estado->regs.RI.imm;
    }
}

void step(struct estado_processador *estado) {
    char aux[INSTR_BITS + 1];

    if (estado->halt_flag) {
        printf("Processador parado (HALT)\n");
        return;
    }

    printf("\nExecutando instrução [PC=%03d]:\n", 
           estado->pc.valor);

    switch(estado->step_atual) {

        case 0: { 
            strncpy(estado->regs.RI.binario, estado->memory.instr_decod[estado->pc.valor].binario, INSTR_BITS);
            estado->regs.RI.binario[INSTR_BITS] = '\0';

            estado->pc.prev_valor = estado->pc.valor;
            estado->pc.valor = ula(ULA_ADD, estado->pc.valor, 1, NULL);

            estado->step_atual = 1;
            break;
        }

        case 1: { 
            armazenaRI(estado);
            estado->regs.A = estado->registradores[estado->regs.RI.rs];
            estado->regs.B = estado->registradores[estado->regs.RI.rt];

            switch (estado->regs.RI.opcode) {
                case 0:  estado->step_atual = 7;  break; // Tipo R
                case 4:  estado->step_atual = 2;  break; // ADDI
                case 11: estado->step_atual = 2;  break; // LW
                case 15: estado->step_atual = 2;  break; // SW
                //case 8:  estado->step_atual = 2;  break; // BEQ
                case 2:  estado->step_atual = 10; break; // JUMP
            }
            break;
        }

        case 2: { 
            switch (estado->regs.RI.opcode) {
                case 4:{ 
                    estado->regs.ULASaida = ula(ULA_ADD, estado->regs.A, estado->regs.RI.imm, NULL);
                    estado->step_atual = 6;
                    break;
                }
                case 11:{ 
                    estado->regs.ULASaida = ula(ULA_ADD, estado->regs.A, estado->regs.RI.imm, NULL);
                    estado->step_atual = 3;
                    break;
                }
                case 15:{ 
                    estado->regs.ULASaida = ula(ULA_ADD, estado->regs.A, estado->regs.RI.imm, NULL);
                    estado->step_atual = 5;
                    break;
                }
                /*case 8: { 
                    estado->regs.ULASaida = ula(ULA_ADD, estado->pc.valor, estado->regs.RI.imm, NULL);
                    estado->step_atual = 9;
                    break;
                }*/
            }
            break;
        }

        case 3: { 
            if (estado->regs.ULASaida >= 0 && estado->regs.ULASaida < MEM_SIZE) {
                estado->regs.RDM = estado->regs.RI.dado;
            }
            estado->step_atual = 4;
            break;
        }

        case 4: { 
            estado->registradores[estado->regs.RI.rt] = estado->regs.RDM;
            estado->step_atual = 0;
            break;
        }

        case 5: { 
            if (estado->regs.ULASaida >= 0 && estado->regs.ULASaida < MEM_SIZE) {
                snprintf(aux, INSTR_BITS + 1, "%016d", estado->regs.B);
                strncpy(estado->memory.instr_decod[estado->regs.ULASaida].binario, aux, INSTR_BITS);
            }
            estado->step_atual = 0;
            break;
        }

        case 6: { 
            estado->registradores[estado->regs.RI.rt] = estado->regs.ULASaida;
            break;
        }

        case 7: { 
            int flag;
            estado->regs.ULASaida = ula(estado->regs.RI.funct, estado->regs.A, estado->regs.B, &flag);
            estado->step_atual = 8;
            break;
        }

        case 8: { 
            estado->registradores[estado->regs.RI.rd] = estado->regs.ULASaida;
            estado->step_atual = 0;
            break;
        }

        /*case 9: { 
            int flag = -1;
            ula(ULA_SUB, estado->regs.A, estado->regs.B, &flag);
            if (flag == BEQ_FLAG) {
                estado->pc.valor = estado->regs.ULASaida;
            }
            estado->step_atual = 0;
            break;
        }*/

        case 10: { 
            estado->pc.valor = estado->regs.RI.addr;
            estado->step_atual = 0;
            break;
        }
    }

    if (estado->pc.valor >= estado->memory.num_instrucoes && estado->step_atual == 0) {
        printf("Fim do programa alcançado\n");
        estado->halt_flag = 1;
    }
}
void criaasm(struct estado_processador *estado, const char *nome_arquivo) {
    char nome_completo[256];
    
    // Verifica se já termina com .asm (case insensitive)
    if (strlen(nome_arquivo) < 4 || (strcasecmp(nome_arquivo + strlen(nome_arquivo) - 4, ".asm") != 0)) {
        snprintf(nome_completo, sizeof(nome_completo), "%s.asm", nome_arquivo);
    } else {
        strncpy(nome_completo, nome_arquivo, sizeof(nome_completo));
    }

    FILE *arqasm = fopen(nome_completo, "w");
    if (!arqasm) {
        printf("Erro ao criar arquivo de saída\n");
        return;
    }
    for (int i = 0; i < estado->memory.num_instrucoes; i++) {
        switch(estado->memory.instr_decod[i].tipo) {
            case tipo_R:
                switch(estado->memory.instr_decod[i].funct) {
                    case 0: fprintf(arqasm, "add $%d, $%d, $%d\n", 
                              estado->memory.instr_decod[i].rd, 
                              estado->memory.instr_decod[i].rs, 
                              estado->memory.instr_decod[i].rt); 
                            break;
                    case 1: fprintf(arqasm, "sub $%d, $%d, $%d\n", 
                              estado->memory.instr_decod[i].rd, 
                              estado->memory.instr_decod[i].rs, 
                              estado->memory.instr_decod[i].rt); 
                            break;
                    case 2: fprintf(arqasm, "and $%d, $%d, $%d\n", 
                              estado->memory.instr_decod[i].rd, 
                              estado->memory.instr_decod[i].rs, 
                              estado->memory.instr_decod[i].rt); 
                            break;
                    case 3: fprintf(arqasm, "or $%d, $%d, $%d\n", 
                              estado->memory.instr_decod[i].rd, 
                              estado->memory.instr_decod[i].rs, 
                              estado->memory.instr_decod[i].rt); 
                            break;
                }
                break;
                
            case tipo_I:
                switch(estado->memory.instr_decod[i].opcode) {
                    case 4: fprintf(arqasm, "addi $%d, $%d, %d\n", 
                              estado->memory.instr_decod[i].rt, 
                              estado->memory.instr_decod[i].rs, 
                              estado->memory.instr_decod[i].imm); 
                            break;
                    case 8: fprintf(arqasm, "lw $%d, %d($%d)\n", 
                              estado->memory.instr_decod[i].rt, 
                              estado->memory.instr_decod[i].imm, 
                              estado->memory.instr_decod[i].rs); 
                            break;
                    case 10: fprintf(arqasm, "sw $%d, %d($%d)\n", 
                              estado->memory.instr_decod[i].rt, 
                              estado->memory.instr_decod[i].imm, 
                              estado->memory.instr_decod[i].rs); 
                            break;
                    case 11: fprintf(arqasm, "beq $%d, $%d, %d\n", 
                              estado->memory.instr_decod[i].rs, 
                              estado->memory.instr_decod[i].rt, 
                              estado->memory.instr_decod[i].imm); 
                            break;
                }
                break;
                
            case tipo_J:
                fprintf(arqasm, "j %d\n", estado->memory.instr_decod[i].addr);
                break;
        }
    }
    
    fclose(arqasm);
    printf("Arquivo assembly gerado com sucesso: %s\n", nome_arquivo);
}

void mostrar_estado_processador(struct estado_processador *estado) {
    printf("\n=== Estado do Processador ===\n");
    printf("PC: %d (anterior: %d)\n", estado->pc.valor, estado->pc.prev_valor);
    printf("Halt flag: %d\n", estado->halt_flag);
    printf("Passos executados: %d\n", estado->passos_executados);
    
    mostrar_registradores(estado->registradores);
    
    print_memory(&estado->memory);
    
    printf("\nPróxima instrução a executar:\n");
    if (estado->pc.valor < estado->memory.num_instrucoes) {
        print_instrucao(&estado->memory.instr_decod[estado->pc.valor]);
    } else {
        printf("Nenhuma (fim do programa alcançado)\n");
    }
}
void salvar_estado_para_arquivo(struct estado_processador *estado, const char *filename) {
    FILE *file = fopen(filename, "a");  
    if (!file) {
        perror("Erro ao abrir arquivo");
        return;
    }

    fprintf(file, "\n=== Estado do Processador ===\n");
    fprintf(file, "PC: %d (anterior: %d)\n", estado->pc.valor, estado->pc.prev_valor);
    fprintf(file, "Halt flag: %d\n", estado->halt_flag);
    fprintf(file, "Passos executados: %d\n", estado->passos_executados);
    
    // Salva registradores
    fprintf(file, "\nRegistradores:\n");
    for (int i = 0; i < REG_COUNT; i++) {
        fprintf(file, "$%d = %d\n", i, estado->registradores[i]);
    }
    
    // Salva memória
    fprintf(file, "\nMemória:\n");
    fprintf(file, "End. | Binário           | Tipo | Opcode | rs | rt | rd | funct | imm  | addr  | Dado\n");
    fprintf(file, "-----------------------------------------------------------------------------------------\n");

    for (int i = 0; i < MEM_SIZE; i++) {
        if (i < estado->memory.num_instrucoes || (i >= DATA_START && estado->memory.instr_decod[i].tipo == tipo_dado)) {
            fprintf(file, "%3d  | %-16s | ", i, estado->memory.instr_decod[i].binario);
            
            switch(estado->memory.instr_decod[i].tipo) {
                case tipo_R:
                    fprintf(file, "R  | %6d | %2d | %2d | %2d | %5d |      |       |\n",
                           estado->memory.instr_decod[i].opcode,
                           estado->memory.instr_decod[i].rs,
                           estado->memory.instr_decod[i].rt,
                           estado->memory.instr_decod[i].rd,
                           estado->memory.instr_decod[i].funct);
                    break;
                case tipo_I:
                    fprintf(file, "I  | %6d | %2d | %2d |    |       | %4d |       |\n",
                           estado->memory.instr_decod[i].opcode,
                           estado->memory.instr_decod[i].rs,
                           estado->memory.instr_decod[i].rt,
                           estado->memory.instr_decod[i].imm);
                    break;
                case tipo_J:
                    fprintf(file, "J  | %6d |    |    |    |       |      | %5d |\n",
                           estado->memory.instr_decod[i].opcode,
                           estado->memory.instr_decod[i].addr);
                    break;
                case tipo_dado:
                    fprintf(file, "DADO |       |    |    |    |       |      |       | %5d\n",
                           estado->memory.instr_decod[i].dado);
                    break;
                default:
                    fprintf(file, "?  | %6d |    |    |    |       |      |       |\n",
                           estado->memory.instr_decod[i].opcode);
            }
        }
    }
    
    // Próxima instrução
    fprintf(file, "\nPróxima instrução a executar:\n");
    if (estado->pc.valor < estado->memory.num_instrucoes) {
        fprintf(file, "Binário: %s\n", estado->memory.instr_decod[estado->pc.valor].binario);
        fprintf(file, "Opcode: %d | Tipo: ", estado->memory.instr_decod[estado->pc.valor].opcode);
        switch (estado->memory.instr_decod[estado->pc.valor].tipo) {
            case tipo_R:
                fprintf(file, "R | rs: %d, rt: %d, rd: %d, funct: %d\n", 
                      estado->memory.instr_decod[estado->pc.valor].rs,
                      estado->memory.instr_decod[estado->pc.valor].rt,
                      estado->memory.instr_decod[estado->pc.valor].rd,
                      estado->memory.instr_decod[estado->pc.valor].funct);
                break;
            case tipo_I:
                fprintf(file, "I | rs: %d, rt: %d, imm: %d\n", 
                      estado->memory.instr_decod[estado->pc.valor].rs,
                      estado->memory.instr_decod[estado->pc.valor].rt,
                      estado->memory.instr_decod[estado->pc.valor].imm);
                break;
            case tipo_J:
                fprintf(file, "J | addr: %d\n", 
                      estado->memory.instr_decod[estado->pc.valor].addr);
                break;
            default:
                fprintf(file, "Inválido\n");
        }
    } else {
        fprintf(file, "Nenhuma (fim do programa alcançado)\n");
    }
    
    fclose(file);
    printf("Estado salvo em '%s'\n", filename);
}
