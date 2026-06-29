#include "lista.h"
#include <stdlib.h>

typedef struct nodo {
    void *elemento;
    struct nodo *siguiente;
} nodo_t;

struct lista {
    nodo_t *cabeza;
    nodo_t *cola;
    size_t tamagno;
};

struct lista_iterador {
    nodo_t *actual;
};

lista_t *lista_crear(void) {
    lista_t *l = calloc(1, sizeof(lista_t));
    return l;
}

void lista_destruir(lista_t *l, void (*destructor)(void*)) {
    if (!l) return;
    nodo_t *n = l->cabeza;
    while (n) {
        nodo_t *sig = n->siguiente;
        if (destructor) destructor(n->elemento);
        free(n);
        n = sig;
    }
    free(l);
}

bool lista_insertar(lista_t *l, void *elemento) {
    if (!l) return false;
    nodo_t *n = malloc(sizeof(nodo_t));
    if (!n) return false;
    n->elemento = elemento;
    n->siguiente = NULL;
    if (!l->cola) {
        l->cabeza = n;
        l->cola = n;
    } else {
        l->cola->siguiente = n;
        l->cola = n;
    }
    l->tamagno++;
    return true;
}

void *lista_primero(const lista_t *l) {
    if (!l || !l->cabeza) return NULL;
    return l->cabeza->elemento;
}

void *lista_ultimo(const lista_t *l) {
    if (!l || !l->cola) return NULL;
    return l->cola->elemento;
}

size_t lista_tamagno(const lista_t *l) { return l ? l->tamagno : 0; }
bool lista_vacia(const lista_t *l) { return !l || l->tamagno == 0; }

lista_iterador_t *lista_iterador_crear(const lista_t *l) {
    if (!l) return NULL;
    lista_iterador_t *li = malloc(sizeof(lista_iterador_t));
    if (!li) return NULL;
    li->actual = l->cabeza;
    return li;
}

bool lista_iterador_tiene_siguiente(const lista_iterador_t *li) {
    return li && li->actual;
}

void *lista_iterador_siguiente(lista_iterador_t *li) {
    if (!li || !li->actual) return NULL;
    void *elem = li->actual->elemento;
    li->actual = li->actual->siguiente;
    return elem;
}

void lista_iterador_destruir(lista_iterador_t *li) {
    free(li);
}
