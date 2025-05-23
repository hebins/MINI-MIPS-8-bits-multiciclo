CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
TARGET = MiniMIPS

SRCS = mainMIPS.c MiniMIPS.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c MiniMIPS.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
