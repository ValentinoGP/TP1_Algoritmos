#ifndef MODELO_H
#define MODELO_H

#include <stddef.h>
#include <stdbool.h>

typedef struct modelo modelo_t;

modelo_t *modelo_crear(const char *nombre, const float *coords, size_t ncoords,
    const size_t *lineas, size_t nlineas);

void modelo_destruir(modelo_t *m);

const char *modelo_nombre(const modelo_t *m);

const float *modelo_coords(const modelo_t *m);

size_t modelo_ncoords(const modelo_t *m);

const size_t *modelo_lineas(const modelo_t *m);

size_t modelo_nlineas(const modelo_t *m);

#endif