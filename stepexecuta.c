#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ================= CONSTANTES E DEFINES =================
#define MEM_SIZE 256       // Tamanho total da memória
#define INSTR_BITS 16      // Tamanho de cada instrução em bits
#define REG_COUNT 8        // Número de registradores
#define DATA_START 128     // Endereço inicial dos dados (alterado para 128)
#define END_OPCODE 255     // Opcode para encerrar execução
#define OVERFLOW_FLAG 1    // Flag de overflow da ULA
#define BEQ_FLAG 0         // Flag de branch equal
#define NO_FLAG -1         // Sem flag

// ================= ESTRUTURAS DE DADOS =================
// Tipos de instruções
enum classe_inst { tipo_R, tipo_I, tipo_J, tipo_dado, tipo_INVALIDO };

// Operações da ULA
enum ops_ula { ULA_ADD, ULA_SUB, ULA_AND, ULA_OR };

typedef struct {
    char instr_mem[MEM_SIZE][INSTR_BITS+1]; // Matriz para armazenar as instruções
    int num_instrucoes;
    int num_dados;       // Contador de dados carregados
} Memory;

// Registrador de Instruções (RI)
struct IR {
    enum classe_inst tipo;
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
    char binario[INSTR_BITS + 1];
    int dado;
};

// Program Counter (PC)
struct pc {
    int valor;
    int prev_valor;
};

typedef struct regs {
    struct IR RI;       // (RI)
    int RDM;            // Registrador de Dados da memória
    int A;              // Reg A (saída do banco de reg)
    int B;              // Reg B (saída do banco de reg)
    int ULASaida;
} REGS;

// Estado completo do processador
struct estado_processador {
    Memory memory;
    int registradores[REG_COUNT];   // Banco de registradores
    struct pc pc;                   
    int halt_flag;                  
    int passos_executados; 
    REGS regs; 
    int step_atual;       
};

// ================= PROTÓTIPOS DE FUNÇÕES =================
void mostrar_memoria_completa(struct estado_processador *cpu);
void load_memory(struct estado_processador *cpu, const char *filename);
void executa_instrucao(struct estado_processador *estado);
int ula(enum ops_ula operacao, int a, int b, int *flag);
void decodificar(const char *inst_str, struct estado_processador *cpu);
void mostrar_registradores(int registradores[]);
void print_instrucao(const struct IR *ri);
void display_menu_principal();
void inicializar_processador(struct estado_processador *cpu);
void display_menu_execucao();
void step(struct estado_processador *estado);
int binario_para_decimal(const char *binario);

// ================= FUNÇÃO PRINCIPAL =================
int main(int argc, char *argv[]) {
    struct estado_processador cpu;
    inicializar_processador(&cpu);
    
    int option;
    char filename[256];
    int em_execucao = 0;
    int sair = 0;

    if (argc > 1) {
        load_memory(&cpu, argv[1]);
    }

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
                case 3: mostrar_memoria_completa(&cpu); break;
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
                    load_memory(&cpu, filename);
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
                    mostrar_memoria_completa(&cpu); 
                    break;
                case 4: break;
                case 5: sair = 1; break;
                default: printf("Opção inválida!\n");
            }
        }
    } while (!sair);

    return 0;
}

// ================= FUNÇÕES AUXILIARES =================

int binario_para_decimal(const char *binario) {
    return (int)strtol(binario, NULL, 2);
}

void mostrar_memoria_completa(struct estado_processador *cpu) {
    printf("\n=== MEMÓRIA COMPLETA ===\n");
    printf("Instruções carregadas: %d\n", cpu->memory.num_instrucoes);
    printf("Dados carregados: %d\n", cpu->memory.num_dados);
    
    // Mostrar seção de instruções (todas as posições)
    printf("\n--- INSTRUÇÕES (0-%d) ---\n", DATA_START-1);
    for (int i = 0; i < DATA_START; i++) {
        printf("[%03d] %s", i, cpu->memory.instr_mem[i]);
        
        if (i == cpu->pc.prev_valor) {
            printf("  <-- Última executada");
        }
        printf("\n");
    }
    
    // Mostrar seção de dados (todas as posições)
    printf("\n--- DADOS (%d-%d) ---\n", DATA_START, MEM_SIZE-1);
    for (int i = DATA_START; i < MEM_SIZE; i++) {
        int valor = binario_para_decimal(cpu->memory.instr_mem[i]);
        printf("[%03d] %s = %d\n", i, cpu->memory.instr_mem[i], valor);
    }
}

void load_memory(struct estado_processador *cpu, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    // Inicializa toda memória com zeros
    for (int i = 0; i < MEM_SIZE; i++) {
        strcpy(cpu->memory.instr_mem[i], "0000000000000000");
    }

    char line[INSTR_BITS+2];
    cpu->memory.num_instrucoes = 0;
    cpu->memory.num_dados = 0;
    int data_mode = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove caracteres especiais
        line[strcspn(line, "\r\n")] = '\0';
        
        // Ignora linhas vazias ou comentários
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }
        
        // Verifica se encontrou o marcador .data
        if (strcmp(line, ".data") == 0) {
            data_mode = 1;
            continue;
        }
        
        // Preenche com zeros à esquerda se necessário
        if (strlen(line) < INSTR_BITS) {
            char temp[INSTR_BITS+1] = {0};
            int zeros = INSTR_BITS - strlen(line);
            
            memset(temp, '0', zeros);
            strcat(temp, line);
            strcpy(line, temp);
        }
        else if (strlen(line) > INSTR_BITS) {
            line[INSTR_BITS] = '\0'; // Trunca se for maior
        }
        
        // Garante que só tem 0s e 1s
        for (char *p = line; *p; p++) {
            if (*p != '0' && *p != '1') {
                *p = '0'; // Substitui caracteres inválidos por 0
            }
        }
        
        if (!data_mode) {
            // Armazenamento de instruções
            if (cpu->memory.num_instrucoes >= DATA_START) {
                printf("Erro: Número de instruções excede o limite de %d\n", DATA_START);
                break;
            }
            strncpy(cpu->memory.instr_mem[cpu->memory.num_instrucoes], line, INSTR_BITS);
            cpu->memory.num_instrucoes++;
        } else {
            // Armazenamento de dados
            if (DATA_START + cpu->memory.num_dados >= MEM_SIZE) {
                printf("Erro: Memória de dados cheia\n");
                break;
            }
            strncpy(cpu->memory.instr_mem[DATA_START + cpu->memory.num_dados], line, INSTR_BITS);
            cpu->memory.num_dados++;
        }
    }
    fclose(file);
    
    printf("Carregadas %d instruções e %d dados na memória\n", 
           cpu->memory.num_instrucoes, cpu->memory.num_dados);
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

void decodificar(const char *inst_str, struct estado_processador *cpu) {
    strncpy(cpu->regs.RI.binario, inst_str, INSTR_BITS);
    cpu->regs.RI.binario[INSTR_BITS] = '\0';

    char opcode_str[5];
    strncpy(opcode_str, inst_str, 4);
    opcode_str[4] = '\0';
    cpu->regs.RI.opcode = strtol(opcode_str, NULL, 2);

    if (cpu->regs.RI.opcode == 0) {
        cpu->regs.RI.tipo = tipo_R;
        char rs_str[4], rt_str[4], rd_str[4], funct_str[4];
        strncpy(rs_str, inst_str + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst_str + 7, 3); rt_str[3] = '\0';
        strncpy(rd_str, inst_str + 10, 3); rd_str[3] = '\0';
        strncpy(funct_str, inst_str + 13, 3); funct_str[3] = '\0';
        cpu->regs.RI.rs = strtol(rs_str, NULL, 2);
        cpu->regs.RI.rt = strtol(rt_str, NULL, 2);
        cpu->regs.RI.rd = strtol(rd_str, NULL, 2);
        cpu->regs.RI.funct = strtol(funct_str, NULL, 2);
    } 
    else if (cpu->regs.RI.opcode == 2) {
        cpu->regs.RI.tipo = tipo_J;
        char addr_str[13];
        strncpy(addr_str, inst_str + 4, 12); addr_str[12] = '\0';
        cpu->regs.RI.addr = strtol(addr_str, NULL, 2);
    } 
    else {
        cpu->regs.RI.tipo = tipo_I;
        char rs_str[4], rt_str[4], imm_str[7];
        strncpy(rs_str, inst_str + 4, 3); rs_str[3] = '\0';
        strncpy(rt_str, inst_str + 7, 3); rt_str[3] = '\0';
        strncpy(imm_str, inst_str + 10, 6); imm_str[6] = '\0';
        cpu->regs.RI.rs = strtol(rs_str, NULL, 2);
        cpu->regs.RI.rt = strtol(rt_str, NULL, 2);
        cpu->regs.RI.imm = strtol(imm_str, NULL, 2);
    }
}

void mostrar_registradores(int registradores[]) {
    printf("\n=== Registradores ===\n");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("$%d = %d\n", i, registradores[i]);
    }
}

void print_instrucao(const struct IR *ri) {
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
    printf("5. Sair\n");
    printf("======================\n");
    printf("Escolha uma opção: ");
}

void display_menu_execucao() {
    printf("\n=== MENU DE EXECUÇÃO ===\n");
    printf("1. Executar próxima instrução\n");
    printf("2. Mostrar registradores\n");
    printf("3. Mostrar memória completa\n");
    printf("4. Mostrar memória de dados\n");
    printf("5. Salvar instruções executadas\n");
    printf("6. Salvar memória de dados\n");
    printf("7. Voltar instrução anterior\n");
    printf("8. Voltar ao menu principal\n");
    printf("9. Mostrar total de instruções executadas\n");
    printf("10. Executar todas as instruções\n");
    printf("=========================\n");
    printf("Escolha uma opção: ");
}

void step(struct estado_processador *estado) {
    int contclock;
    contclock = 1;

    if (estado->halt_flag) {
        printf("Processador parado (HALT)\n");
        return;
    }

    printf("\nExecutando instrução [PC=%03d]:\nCiclo de clock atual: %d\n", 
           estado->pc.valor, contclock;);

    switch(estado->step_atual) {
        case 0: {
            strncpy(estado->regs.RI.binario, estado->memory.instr_mem[estado->pc.valor], INSTR_BITS);
            estado->regs.RI.binario[INSTR_BITS] = '\0';

            estado->pc.prev_valor = estado->pc.valor;
            estado->pc.valor = ula(ULA_ADD, estado->pc.valor, 1, NULL);

            estado->step_atual=1;
            contclock++;
            break;
        }

        case 1: {
            decodificar(estado->memory.instr_mem[estado->pc.prev_valor], estado);
            estado->regs.A = estado->registradores[estado->regs.RI.rs];
            estado->regs.B = estado->registradores[estado->regs.RI.rt];

            switch(estado->regs.RI.opcode){
                case 0: { //R
                    estado->step_atual=7;
                    break;
                }
                case 4: { //ADDI
                    estado->step_atual=2;
                    break;
                }
                case 11: { //LW
                    estado->step_atual=2;
                    break;
                }
                case 15: {  //SW
                    estado->step_atual=2;
                    break;
                } 
                case 8: { //BEW
                    estado->step_atual=9;
                    break;
                }
                case 2: { //JUMP
                    estado->step_atual=10;
                    break;
                }
            }
            contclock++;
            break;
        }

        case 2: {
            estado->regs.ULASaida = ula(ULA_ADD, estado->regs.A, estado->regs.RI.imm, NULL);
            
            switch(estado->regs.RI.opcode){
                case 4: {
                    estado->step_atual=6;
                    break;
                }
                case 11: {
                    estado->step_atual=3;
                    break;
                }
                case 15: {
                    estado->step_atual=5;
                    break;
                }
            }
            contclock++;
            break;
        }
        case 3: {
            if (estado->regs.ULASaida >= 0 && estado->regs.ULASaida < MEM_SIZE) {
                estado->regs.RDM = binario_para_decimal(estado->memory.instr_mem[estado->regs.ULASaida]);
            }
            estado->step_atual = 4;
            contclock++;
            break;
        }
        case 4: {
            estado->registradores[estado->regs.RI.rt] = estado->regs.RDM;
            estado->step_atual = 0;
            contclock++;
            break;
        }
        case 5: {
            if (estado->regs.ULASaida >= 0 && estado->regs.ULASaida < MEM_SIZE) {
                    char temp[INSTR_BITS+1];
                    snprintf(temp, INSTR_BITS+1, "%016d", estado->regs.B);
                    strncpy(estado->memory.instr_mem[estado->regs.ULASaida], temp, INSTR_BITS);
                }
            estado->step_atual = 0;
            contclock++;
            break;
        }
        case 6: {
            estado->registradores[estado->regs.RI.rd] = estado->regs.ULASaida;
            estado->step_atual = 0;
            contclock++;
            break;
        }
        case 7: {
            int flag;
            estado->regs.ULASaida = ula(estado->regs.RI.funct, estado->regs.A, estado->regs.B, &flag);

            estado->step_atual = 8;
            contclock++;
            break;
        }
        case 8: {
            estado->registradores[estado->regs.RI.rd] = estado->regs.ULASaida;

            estado->step_atual = 0;
            contclock++;
            break;
        }
        case 9: { //BEQ

            estado->step_atual = 0;
            contclock++;
            break;
        }
        case 10: { //jump
            estado->step_atual = 10;
            contclock++;
            break;
        }
    }
    
    if(estado->pc.valor >= estado->memory.num_instrucoes && estado->step_atual == 0) {
        printf("Fim do programa alcançado\n");
        estado->halt_flag = 1;
    }
}
