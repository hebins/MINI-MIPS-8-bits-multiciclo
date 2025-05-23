# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -Wall

# Nome do executável e arquivos
EXEC = MiniMIPS
SRCS = src/mainMIPS.c src/MiniMIPS.c
OBJS = $(SRCS:.c=.o)
HEADERS = include/MiniMIPS.h

# Regras
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC)

.PHONY: all clean run