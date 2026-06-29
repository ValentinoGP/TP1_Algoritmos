#ifndef PILA_H
#define PILA_H

#include <stdbool.h>
#include <stddef.h>

typedef struct pila pila_t;

pila_t  *pila_crear(void);
void     pila_destruir(pila_t *p);
bool     pila_apilar(pila_t *p, void *elemento);
void    *pila_tope(const pila_t *p);
void    *pila_desapilar(pila_t *p);
size_t   pila_tamagno(const pila_t *p);
bool     pila_vacia(const pila_t *p);

#endif
