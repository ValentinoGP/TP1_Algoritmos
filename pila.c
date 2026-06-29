#include "pila.h"
#include <stdlib.h>

#define CAP_INICIAL 32

struct pila {
    void **elementos;
    size_t tamagno;
    size_t capacidad;
};

pila_t *pila_crear(void) {
    pila_t *p = malloc(sizeof(pila_t));
    if (!p) return NULL;
    p->elementos = malloc(CAP_INICIAL * sizeof(void*));
    if (!p->elementos) { free(p); return NULL; }
    p->tamagno = 0;
    p->capacidad = CAP_INICIAL;
    return p;
}

void pila_destruir(pila_t *p) {
    if (!p) return;
    free(p->elementos);
    free(p);
}

bool pila_apilar(pila_t *p, void *elemento) {
    if (p->tamagno >= p->capacidad) {
        size_t nueva_cap = p->capacidad * 2;
        void **nuevos = realloc(p->elementos, nueva_cap * sizeof(void*));
        if (!nuevos) return false;
        p->elementos = nuevos;
        p->capacidad = nueva_cap;
    }
    p->elementos[p->tamagno++] = elemento;
    return true;
}

void *pila_tope(const pila_t *p) {
    if (p->tamagno == 0) return NULL;
    return p->elementos[p->tamagno - 1];
}

void *pila_desapilar(pila_t *p) {
    if (p->tamagno == 0) return NULL;
    return p->elementos[--p->tamagno];
}

size_t pila_tamagno(const pila_t *p) { return p->tamagno; }
bool pila_vacia(const pila_t *p) { return p->tamagno == 0; }
