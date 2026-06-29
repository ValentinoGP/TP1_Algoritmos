CC = gcc
CFLAGS = -I/opt/homebrew/include -g -Wall -Wextra -pedantic
LDFLAGS = -L/opt/homebrew/lib -lSDL2main -lSDL2 -lm -Wl,-framework,Cocoa
SRCS = main.c modelo.c obstaculo.c tanque.c stl.c matriz.c pila.c lista.c
OBJS = $(SRCS:.c=.o)
TARGET = battlezone

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
