#ifndef LISTA_H
#define LISTA_H

#include <stdbool.h>
#include <stddef.h>

typedef struct lista lista_t;
typedef struct lista_iterador lista_iterador_t;

lista_t *lista_crear(void);
void     lista_destruir(lista_t *l, void (*destructor)(void*));
bool     lista_insertar(lista_t *l, void *elemento);
void    *lista_primero(const lista_t *l);
void    *lista_ultimo(const lista_t *l);
size_t   lista_tamagno(const lista_t *l);
bool     lista_vacia(const lista_t *l);

lista_iterador_t *lista_iterador_crear(const lista_t *l);
bool              lista_iterador_tiene_siguiente(const lista_iterador_t *li);
void             *lista_iterador_siguiente(lista_iterador_t *li);
void              lista_iterador_destruir(lista_iterador_t *li);

#endif
