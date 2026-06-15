#include "modelo.h"
#include <stdlib.h>
#include <string.h>

struct modelo {
    char *nombre;
    float *coords;
    size_t ncoords;
    size_t *lineas;
    size_t nlineas;
};

modelo_t *modelo_crear(const char *nombre, const float *coords, size_t ncoords,
                        const size_t *lineas, size_t nlineas) {
    modelo_t *m = malloc(sizeof(modelo_t));
    if (m == NULL) return NULL;

    m->nombre = malloc(strlen(nombre) + 1);
    if (m->nombre == NULL) { free(m); return NULL; }
    strcpy(m->nombre, nombre);

    m->coords = malloc(sizeof(float) * 3 * ncoords);
    if (m->coords == NULL) { free(m->nombre); free(m); return NULL; }
    memcpy(m->coords, coords, sizeof(float) * 3 * ncoords);

    m->lineas = malloc(sizeof(size_t) * 2 * nlineas);
    if (m->lineas == NULL) { free(m->coords); free(m->nombre); free(m); return NULL; }
    memcpy(m->lineas, lineas, sizeof(size_t) * 2 * nlineas);

    m->ncoords = ncoords;
    m->nlineas = nlineas;
    return m;
}

void modelo_destruir(modelo_t *m) {
    free(m->nombre);
    free(m->coords);
    free(m->lineas);
    free(m);
}

const char *modelo_nombre(const modelo_t *m) { 
    return m->nombre; 
}

const float *modelo_coords(const modelo_t *m) { 
    return m->coords; 
}

size_t modelo_ncoords(const modelo_t *m) { 
    return m->ncoords; 
}

const size_t *modelo_lineas(const modelo_t *m) { 
    return m->lineas; 
}

size_t modelo_nlineas(const modelo_t *m) { 
    return m->nlineas; 
}