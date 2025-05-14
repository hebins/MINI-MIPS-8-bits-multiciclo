#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ================= CONSTANTES E DEFINES =================
#define MEM_SIZE 256       // Tamanho total da memória
#define INSTR_BITS 16      // Tamanho de cada instrução em bits
#define REG_COUNT 8         // Número de registradores
#define DATA_START 129      // Endereço inicial dos dados
#define END_OPCODE 255      // Opcode para encerrar execução
#define OVERFLOW_FLAG 1     // Flag de overflow da ULA
#define BEQ_FLAG 0          // Flag de branch equal
#define NO_FLAG -1          // Sem flag

// ================= ESTRUTURAS DE DADOS =================
// Tipos de instruções
enum classe_inst { tipo_R, tipo_I, tipo_J, tipo_dado, tipo_INVALIDO };

// Operações da ULA
enum ops_ula { ULA_ADD, ULA_SUB, ULA_AND, ULA_OR };

typedef struct {
    char instr_mem[MEM_SIZE][INSTR_BITS+1]; // Matriz para armazenar as instruções
    int num_instrucoes;
} Memory;

// Registrador de Instruções (RI)
struct IR{
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

typedef struct regs{
    struct IR RI;       //(RI)
    int RDM;            //Registrador de Dados da memória
    int A;              //Reg A (saída do banco de reg)
    int B;              //Reg B (saída do banco de reg)
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

// ================= FUNÇÃO PRINCIPAL =================
int main() {
    struct estado_processador cpu;
    inicializar_processador(&cpu);
    
    int option;
    char filename[256];
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
                case 3: /* imprimir_memoria_instrucoes(&cpu.mem_instrucoes); */ break;
                case 4: /* mostrar_memoria_dados(&cpu.mem_dados); */ break;
                case 5: /* salvar_instrucoes_executadas(...); */ break;
                case 6: /* salvar_memoria_dados(...); */ break;
                case 7: /* back(&cpu); */ break;
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
                case 3: /* mostrar_estado_processador(&cpu); */ break;
                case 4: /* salvar_estado_para_arquivo(&cpu, filename); */ break;
                case 5: sair = 1; break;
                default: printf("Opção inválida!\n");
            }
        }
    } while (!sair);

    return 0;
}

// Carrega instruções na memória
void load_memory(struct estado_processador *cpu, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    char line[INSTR_BITS+2];
    cpu->memory.num_instrucoes = 0;
    
    while (fgets(line, sizeof(line), file) && cpu->memory.num_instrucoes < MEM_SIZE) {
        line[strcspn(line, "\n")] = '\0';
        
        if(strlen(line)<INSTR_BITS){
            int zeros = INSTR_BITS - strlen(line);
            char temp[INSTR_BITS+1] = {0};
            
            for (int j=0; j<zeros;j++){
                temp[j]='0';
            }
            strcat(temp,line);
            strcpy(line,temp);
        }
        
        if (strlen(line) == INSTR_BITS) {
            strncpy(cpu->memory.instr_mem[cpu->memory.num_instrucoes], line, INSTR_BITS);
            cpu->memory.instr_mem[cpu->memory.num_instrucoes][INSTR_BITS] = '\0';
            cpu->memory.num_instrucoes++;
        }
    }
    fclose(file);
}

// executa instrução
void executa_instrucao(struct estado_processador *estado) {
    switch (estado->step_atual) {
        case 2:
            if (estado->regs.RI.opcode == 0) { // Tipo R
                int flag;
                estado->regs.ULASaida = ula(estado->regs.RI.funct, estado->regs.A, estado->regs.B, &flag);
            }
            else if (estado->regs.RI.opcode == 8) { // ADDI
                estado->regs.ULASaida = ula(ULA_ADD, estado->regs.A, estado->regs.RI.imm, NULL);
            }
            else if (estado->regs.RI.opcode == 11) { // LW
                estado->regs.ULASaida = ula(ULA_ADD, estado->regs.A, estado->regs.RI.imm, NULL);
            } 
            else if (estado->regs.RI.opcode == 15) { // SW
                estado->regs.ULASaida = ula(ULA_ADD, estado->regs.A, estado->regs.RI.imm, NULL);
            }
            break;

        case 3:
            if (estado->regs.RI.opcode == 0 || estado->regs.RI.opcode == 8) { // Tipo R ou ADDI
                estado->registradores[estado->regs.RI.rd] = estado->regs.ULASaida;
            }
            else if (estado->regs.RI.opcode == 11) { // LW
                if (estado->regs.ULASaida >= 0 && estado->regs.ULASaida < MEM_SIZE) {
                    estado->regs.RDM = atoi(estado->memory.instr_mem[estado->regs.ULASaida]);
                }
            } else if(estado->regs.RI.opcode == 15) { // SW
                if (estado->regs.ULASaida >= 0 && estado->regs.ULASaida < MEM_SIZE) {
                    sprintf(estado->memory.instr_mem[estado->regs.ULASaida], "%d", estado->regs.B);
                }
            }
            break;

        case 4:
            if (estado->regs.RI.opcode == 11) { // LW
                estado->registradores[estado->regs.RI.rt] = estado->regs.RDM;
            }
            break;
    }
}

// Executa operações na ULA
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

// Decodifica instruções
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

// ================= FUNÇÕES AUXILIARES =================
void mostrar_registradores(int registradores[]) {
    printf("\n=== Registradores ===\n");
    for (int i = 0; i < REG_COUNT; i++) {
        printf("$%d = %d\n", i, registradores[i]);
    }
}

void print_instrucao(const struct IR *ri) {
    printf("Binário: %s\n", ri->binario);
    printf("Opcode: %d | Tipo: ", ri->opcode);
    switch (ri->tipo){
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
    printf("3. Mostrar memória de instruções\n");
    printf("4. Mostrar memória de dados\n");
    printf("5. Salvar instruções executadas em arquivo\n");
    printf("6. Salvar memória de dados em arquivo\n");
    printf("7. Voltar instrução anterior\n");
    printf("8. Voltar ao menu principal\n");
    printf("9. Mostrar total de instruções executadas\n");
    printf("10. Executar todas as instruções até o fim\n");
    printf("=========================\n");
    printf("Escolha uma opção: ");
}

void step(struct estado_processador *estado) {
    if (estado->halt_flag) {
        printf("Processador parado (HALT)\n");
        return;
    }

    printf("\nExecutando instrução [PC=%03d]:\nCiclo de clock atual: %d\n", 
           estado->pc.valor, estado->step_atual);

    switch(estado->step_atual) {
        case 0: {
            strncpy(estado->regs.RI.binario, estado->memory.instr_mem[estado->pc.valor], INSTR_BITS);
            estado->regs.RI.binario[INSTR_BITS] = '\0';

            estado->pc.prev_valor = estado->pc.valor;
            estado->pc.valor = ula(ULA_ADD, estado->pc.valor, 1, NULL);

            estado->step_atual++;
            break;
        }

        case 1: {
            decodificar(estado->memory.instr_mem[estado->pc.prev_valor], estado);
            estado->regs.A = estado->registradores[estado->regs.RI.rs];
            estado->regs.B = estado->registradores[estado->regs.RI.rt];
            estado->step_atual++;
            break;
        }

        case 2: {
            executa_instrucao(estado);
            estado->step_atual++;
            break;
        }
        case 3: {
            executa_instrucao(estado);
            if (estado->regs.RI.opcode == 11) { // LW precisa de passo extra
                estado->step_atual++;
            } else {
                estado->step_atual = 0;
            }
            break;
        }
        case 4: {
            executa_instrucao(estado);
            estado->step_atual = 0;
            break;
        }
    }

    estado->passos_executados++;
    
    if(estado->pc.valor >= estado->memory.num_instrucoes && estado->step_atual == 0){
        printf("Fim do programa alcançado\n");
        estado->halt_flag = 1;
    }
}
