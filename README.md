# MINI-MIPS-8-bits-multiciclo

## 🧠 MINI MIPS 8 bits Multicíclo - Simulador em C

Este repositório contém um simulador desenvolvido em linguagem C para um processador MINI MIPS de 8 bits com arquitetura multicíclo. O projeto tem como objetivo auxiliar no entendimento do funcionamento interno de processadores baseados na arquitetura MIPS, explorando conceitos como estados do controle, datapath e execução multicíclica de instruções.

## 🚀 Características

- Simulação de um processador MINI MIPS com barramento de dados de 8 bits
- Arquitetura multicíclo, com estados de controle definidos por etapas da instrução
- Implementação de um subconjunto de instruções MIPS (ex: `add`, `sub`, `lw`, `sw`, `beq`, etc.)
- Organização modular do código (UC - Unidade de Controle, ULA, Banco de Registradores, Memória, etc.)
- Saída detalhada do estado do processador a cada ciclo
- Fácil modificação e extensão para fins educacionais

## 🧩 Instruções Suportadas

| Tipo | Mnêmico | Descrição                 |
|------|---------|---------------------------|
| R    | `add`   | Soma registradores        |
| R    | `sub`   | Subtração                 |
| R    | `and`   | Operação AND lógica       |
| R    | `or`    | Operação OR lógica        |
| I    | `addi`  | Soma imediata             |
| I    | `lw`    | Load word (carrega da memória) |
| I    | `sw`    | Store word (salva na memória) |
| I    | `beq`   | Branch se igual           |
| J    | `j`     | Salto incondicional       |

> As instruções seguem o formato binário de 16 bits, com campos para opcode, registradores e imediato/endereço conforme o tipo.

## 🎓 Objetivos Educacionais
Este projeto foi desenvolvido como ferramenta de aprendizado para disciplinas como Arquitetura e Organização de Computadores e Algoritmos e Estruturas de Dados. Ele visa facilitar a visualização da execução passo a passo das instruções em uma arquitetura de controle multicíclo.

## 📁 Organização do Código

- `mainMIPS.c` – Função principal
- `MiniMIPS.h` – Estrutura e lógica do multiciclo
- `MiniMIPS.c` – Funções auxiliares de execução do simulador Mini MIPS
- `makefile` – Makefile para compilação da TAD

## 🔧 Como Compilar

Digite no terminal:
make

## 📄 Licença
Este projeto está licenciado sob a MIT License.
