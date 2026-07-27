CC = gcc

CFLAGS = -Wall -Werror -std=c99 -pedantic -g \
         $(shell sdl2-config --cflags | sed 's|/SDL2||')

LDLIBS = $(shell sdl2-config --libs) -lm

OBJS = matriz.o modelo.o obstaculo.o tanque.o stl.o pila.o lista.o cola.o juego.o main.o

PROGRAM = battlezone

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

matriz.o: matriz.c matriz.h
modelo.o: modelo.c modelo.h matriz.h
obstaculo.o: obstaculo.c obstaculo.h modelo.h
tanque.o: tanque.c tanque.h obstaculo.h modelo.h
stl.o: stl.c stl.h
pila.o: pila.c pila.h
lista.o: lista.c lista.h
cola.o: cola.c cola.h
juego.o: juego.c juego.h modelo.h obstaculo.h tanque.h stl.h matriz.h pila.h lista.h cola.h
main.o: main.c juego.h modelo.h obstaculo.h matriz.h pila.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROGRAM)

.PHONY: all clean
