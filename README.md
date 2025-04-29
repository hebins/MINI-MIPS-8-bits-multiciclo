# MINI-MIPS-8-bits-multiciclo

# 🧠 MINI MIPS 8 bits Multicíclo - Simulador em C

Este repositório contém um simulador desenvolvido em linguagem C para um processador MINI MIPS de 8 bits com arquitetura multicíclo. O projeto tem como objetivo auxiliar no entendimento do funcionamento interno de processadores baseados na arquitetura MIPS, explorando conceitos como estados do controle, datapath e execução multicíclica de instruções.

## 🚀 Características

- Simulação de um processador MINI MIPS com barramento de dados de 8 bits
- Arquitetura multicíclo, com estados de controle definidos por etapas da instrução
- Implementação de um subconjunto de instruções MIPS (ex: `add`, `sub`, `lw`, `sw`, `beq`, etc.)
- Organização modular do código (UC - Unidade de Controle, ULA, Banco de Registradores, Memória, etc.)
- Saída detalhada do estado do processador a cada ciclo
- Fácil modificação e extensão para fins educacionais

## 📁 Estrutura do Projeto

📘 Instruções Suportadas
O simulador suporta um subconjunto de instruções MIPS. Exemplos:

R-Type: add, sub, and, or

I-Type: lw, sw, beq, addi

Jumps: j (se implementado)

🎓 Objetivos Educacionais
Este projeto foi desenvolvido como ferramenta de aprendizado para disciplinas como Organização de Computadores e Arquitetura de Processadores. Ele visa facilitar a visualização da execução passo a passo das instruções em uma arquitetura de controle multicíclo.

📄 Licença
Este projeto está licenciado sob a MIT License.
