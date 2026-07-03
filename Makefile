CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic
LDFLAGS = -lSDL2 -lm
SRCS = main.c modelo.c obstaculo.c tanque.c stl.c matriz.c pila.c lista.c
TARGET = battlezone

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
