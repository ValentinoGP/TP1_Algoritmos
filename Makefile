CC = gcc
SDL_CFLAGS := $(shell sdl2-config --cflags | sed 's|/SDL2||')
CFLAGS = -g -Wall -Wextra -pedantic $(SDL_CFLAGS)
LDFLAGS = $(shell sdl2-config --libs) -lm
SRCS = main.c modelo.c obstaculo.c tanque.c stl.c matriz.c pila.c lista.c cola.c
TARGET = battlezone

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
