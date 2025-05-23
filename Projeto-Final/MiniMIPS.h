// Arquivo: MiniMIPS.h
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

typedef struct regs {
    struct inst_dados RI;
    int RDM;
    int A;
    int B;
    int ULASaida;
} REGS;

struct estado_salvo {
    struct inst_dados RI;
    int RDM;
    int A;
    int B;
    int ULASaida;
    int pc;
    int registradores[REG_COUNT];
    Memory memory;
    int step_atual;
};

typedef struct node {
    struct estado_salvo estado;
    struct node* next;
} Node;

typedef struct {
    Node* top;
    int size;
} Stack;

struct estado_processador {
    Memory memory;
    int registradores[REG_COUNT];
    int pc;
    int pc_prev;
    int halt_flag;
    int passos_executados;
    REGS regs;
    int step_atual;
    Stack historico_stack;
};

// ================= PROTÓTIPOS DE FUNÇÕES =================
void load_memory(Memory *memory, const char *filename);
int ula(enum ops_ula operacao, int a, int b, int *flag);
void mostrar_registradores(int registradores[]);
void print_instrucao(const struct inst_dados *ri);
void display_menu_principal();
void inicializar_processador(struct estado_processador *cpu);
void step(struct estado_processador *estado);
void int_para_binario(int valor, char *saida);
void armazenaRI(struct estado_processador *estado);
void criaasm(struct estado_processador *estado, const char *nome_arquivo);
void salvar_estado_para_arquivo(struct estado_processador *estado, const char *filename);
void mostrar_estado_processador(struct estado_processador *estado);
int step_back(struct estado_processador *estado);
void init_stack(Stack *s);
void push(Stack *s, struct estado_processador *estado);
int pop(Stack *s, struct estado_salvo *estado_saida);
int vazio(Stack *s);
void salvar_memoria_recarregavel(struct estado_processador *estado, const char *filename);
void print_memory(const Memory *memory);
void salvar_memoria_arquivo(struct estado_processador *estado, const char *filename);
void mostraregs(struct estado_processador *estado);
void printinst(struct inst_dados *ri);
