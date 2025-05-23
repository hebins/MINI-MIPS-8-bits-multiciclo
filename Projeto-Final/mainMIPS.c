#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MiniMIPS.h"

// ================= FUNÇÃO PRINCIPAL =================
int main(void) {
    struct estado_processador cpu;
    char filename[256];
    inicializar_processador(&cpu);
    
    int option;
    int sair = 0;

    do {
       
        display_menu_principal();
        scanf("%d", &option);
        getchar();

            switch(option) {
                case 1:
                    printf("Digite o nome do arquivo de instruções: ");
                    fgets(filename, sizeof(filename), stdin);
                    filename[strcspn(filename, "\n")] = '\0';
                    load_memory(&cpu.memory, filename);
                    break;
                case 2:
                    if (cpu.memory.num_instrucoes > 0) {
                        cpu.halt_flag = 0;
                        step(&cpu);
                    } else {
                        printf("Erro: Nenhuma instrução carregada\n");
                    }
                    break;
                case 3: 
                    step_back(&cpu);
					break;
                case 4: 
					if (cpu.memory.num_instrucoes == 0) {
                        printf("Erro: Nenhuma instrução carregada\n");
                        break;
                    }
                    
                    cpu.halt_flag = 0; // Garante que o processador está executando
                    while (cpu.halt_flag == 0) { 
                        step(&cpu);
                    }
                    break;
                case 5:
                    mostrar_estado_processador(&cpu);
                    break;
                case 6:
                    mostrar_registradores(cpu.registradores);
                    break;
                case 7:
                    print_memory(&cpu.memory);
                    break;
                case 8:
                    printf("Total de instruções executadas: %d\n", cpu.passos_executados);
                    break;
                case 9:
                    printf("Digite o nome do arquivo para salvar: ");
					char filename[256];
					fgets(filename, sizeof(filename), stdin);
					filename[strcspn(filename, "\n")] = '\0';
					salvar_estado_para_arquivo(&cpu, filename);
                    break;
                case 10:
                    printf("Digite o nome do arquivo para salvar: ");
				    fgets(filename, sizeof(filename), stdin);
				    filename[strcspn(filename, "\n")] = '\0';
				    salvar_memoria_arquivo(&cpu, filename);
                    break;
                case 11:
                    printf("Digite o nome do arquivo para salvar: ");
				    fgets(filename, sizeof(filename), stdin);
				    filename[strcspn(filename, "\n")] = '\0';
				    salvar_memoria_recarregavel(&cpu, filename);
				    break;
                case 12:
                    printf("Digite o nome do arquivo para salvar: ");
				    fgets(filename, sizeof(filename), stdin);
				    filename[strcspn(filename, "\n")] = '\0';
				    salvar_memoria_recarregavel(&cpu, filename);
				    break;
                    if (cpu.memory.num_instrucoes > 0) {
                        printf("Digite o nome do arquivo de saída (.asm): ");
                        fgets(filename, sizeof(filename), stdin);
                        filename[strcspn(filename, "\n")] = '\0';
                        criaasm(&cpu, filename);
                    } else {
                        printf("Erro: Nenhuma instrução carregada para converter\n");
                    }
                    break;
                case 13: 
                    sair = 1; 
                    break;
                default: printf("Opção inválida!\n");
            }
    } while (!sair);

    return 0;
}