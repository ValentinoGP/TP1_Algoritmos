CC = gcc

CFLAGS = -Wall -Werror -std=c99 -pedantic -g \
         $(shell sdl2-config --cflags | sed 's|/SDL2||')

LDLIBS = $(shell sdl2-config --libs) -lm

OBJS = matriz.o modelo.o obstaculo.o tanque.o misil.o stl.o pila.o lista.o juego.o dibujo.o main.o

PROGRAM = battlezone

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

matriz.o: matriz.c matriz.h
modelo.o: modelo.c modelo.h matriz.h
obstaculo.o: obstaculo.c obstaculo.h modelo.h
tanque.o: tanque.c tanque.h obstaculo.h modelo.h
misil.o: misil.c misil.h
stl.o: stl.c stl.h
pila.o: pila.c pila.h
lista.o: lista.c lista.h
juego.o: juego.c juego.h modelo.h obstaculo.h tanque.h misil.h stl.h lista.h
dibujo.o: dibujo.c dibujo.h juego.h modelo.h obstaculo.h matriz.h pila.h
main.o: main.c juego.h dibujo.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROGRAM)

.PHONY: all clean
