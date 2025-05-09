

void executa_proxima_instrucao(struct estado_processador *estado) {
    if (estado->halt_flag) {
        printf("Processador parado (HALT)\n");
        return;
    }

    switch (estado->step_atual) {
        // ============= PASSO 0: BUSCA DA INSTRUÇÃO (FETCH) =============
        case 0: {
            // 1. Busca da instrução (RI = Mem[PC])
            strncpy(estado->RI.binario, estado->mem_instrucoes.instr_mem[estado->pc.valor], INSTR_BITS);
            estado->RI.binario[INSTR_BITS] = '\0';

            // 2. Incrementa PC (PC = PC + 1)
            estado->pc.prev_valor = estado->pc.valor;
            estado->pc.valor = ula(ULA_ADD, estado->pc.valor, 1, NULL);

            estado->step_atual = 1;  // Próximo passo: DECODE
            break;
        }

        // ============= PASSO 1: DECODIFICAÇÃO (DECODE) =============
        case 1: {
            // Decodifica a instrução (opcode, rs, rt, rd, funct)
            decodificar(estado->RI.binario, &estado->RI, estado);

            // Lê registradores rs e rt (A = Reg[rs], B = Reg[rt])
            estado->A = estado->registradores[estado->RI.rs];
            estado->B = estado->registradores[estado->RI.rt];

            estado->step_atual = 2;  // Próximo passo: EXECUTE
            break;
        }

        // ============= PASSO 2: EXECUÇÃO (EXECUTE - TIPO R) =============
        case 2: {
            // ------ SOMENTE TIPO R (opcode 0) ------
            if (estado->RI.opcode == 0) {
                int flag;
                // Executa operação na ULA (ADD, SUB, AND, OR) conforme funct
                estado->ULASaida = ula(estado->RI.funct, estado->A, estado->B, &flag);
                estado->step_atual = 7;  // Próximo passo: WB
            } else if(estado->RI.opcode == 15){ //SW
                estado->ULASaida = estado->A + ((short)(estado->RI & 0x3F) << 10 >> 10);
                
                estado->step_atual = 5;
            }
            break;
        }

        // ============= PASSO 3: WRITE-BACK (WB - TIPO R) =============
        case 7: {
            // Escreve o resultado no registrador destino (Reg[rd] = ULASaida)
            estado->registradores[estado->RI.rd] = estado->ULASaida;
            estado->step_atual = 0;  // Reinicia ciclo
            break;
        }

        // ============= PASSOS 4+ (COMENTADOS PARA OUTRAS INSTRUÇÕES) =============
        case 5: {
            estado->mem_dados[estado->ULASaida] = estado->B;

            estado->step_atual = 0;
        }
        /*
        case 4: {  // Para LW/SW (seus colegas implementam)
            break;
        }
        */
    }

    // Debug (opcional)
    printf("Step %d | PC=%d | A=%d | B=%d | ULA=%d\n",
           estado->step_atual, estado->pc.valor, estado->A, estado->B, estado->ULASaida);
}
