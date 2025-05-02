#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256       // Tamanho total da memória
#define INSTR_BITS 16      // Tamanho de cada instrução em bits
#define DATA_START 129     //Endereço dos dados

typedef struct {
    char instr_mem[MEM_SIZE][INSTR_BITS+1]; // Matriz para armazenar as instruções
} Memory;

void load_memory(Memory *memory, const char *filename) {
    FILE *file = fopen(filename, "r");  // Abre o arquivo no modo leitura
    if (!file) {
        perror("Erro ao abrir arquivo"); // Se falhar, mostra erro
        exit(1);                        // E encerra o programa
    }

    char line[INSTR_BITS+2]; // Buffer para ler cada linha (+2 para \n e \0)
    int i = 0;               // Contador de linhas
    
    // Lê cada linha do arquivo até preencher a memória ou acabar o arquivo
    while (fgets(line, sizeof(line), file) && i < MEM_SIZE) {
        line[strcspn(line, "\n")] = '\0'; // Remove o caractere de nova linha
        
        if(strlen(line)<INSTR_BITS){
			int zeros = INSTR_BITS - strlen(line);
			char temp[INSTR_BITS+1] = {0}; //temporário
			
			//adicionar 0 á esquerda
			for (int j=0; j<zeros;j++){
			    temp[j]='0';
			}
			strcat(temp,line);
			strcpy(line,temp);
		}
        
        // Verifica se a linha tem exatamente 16 caracteres
        if (strlen(line) == INSTR_BITS) {
            // Copia a instrução para a matriz de memória
            strncpy(memory->instr_mem[i], line, INSTR_BITS);
            memory->instr_mem[i][INSTR_BITS] = '\0'; // Garante o terminador nulo
            i++;
        }
    }
    fclose(file); // Fecha o arquivo
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_instrucoes.txt>\n", argv[0]);
        return 1;
    }

    Memory mem = {0};       // Cria a estrutura de memória
    load_memory(&mem, argv[1]); // Carrega o arquivo na memória

    // Imprime as primeiras 3 instruções carregadas
    for (int i = 0; i < 3 && mem.instr_mem[i][0] != '\0'; i++) {
        printf("Instrução %d: %s\n", i, mem.instr_mem[i]);
    }

    return 0;
}
