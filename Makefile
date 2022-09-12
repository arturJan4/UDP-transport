CC = gcc -g

DEBUGFLAGS = -fsanitize=address -fsanitize=leak -fsanitize=undefined -static-libasan

CFLAGS = -O2 -std=gnu17 -Wall -Wextra -Werror
#CFLAGS = -O2 -std=gnu17 $(DEBUGFLAGS)

OBJS = main.o input.o receive.o send.o segment.o 

all: transport

transport: $(OBJS)
	$(CC) $(CFLAGS) -o transport $(OBJS)

input.o: input.c input.h
send.o: send.c send.h
receive.o: receive.c receive.h
segment.o: segment.c segment.h

SRC_C = $(wildcard *.c)
SRC_H = $(wildcard *.h)

clean:
	rm -f *.o

distclean:
	rm -f *.o transport

format:
	clang-format --style=Google -i $(SRC_C) $(SRC_H)