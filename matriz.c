#include "matriz.h"
#include <stdlib.h>
#include <math.h>

struct matriz {
    float *m;
    size_t filas, columnas;
};

matriz_t *_matriz_crear(size_t n, size_t m) {
    matriz_t *mat = malloc(sizeof(matriz_t));
    if (!mat) return NULL;
    mat->m = calloc(n * m, sizeof(float));
    if (!mat->m) { free(mat); return NULL; }
    mat->filas = n;
    mat->columnas = m;
    return mat;
}

void matriz_destruir(matriz_t *matriz) {
    if (!matriz) return;
    free(matriz->m);
    free(matriz);
}

size_t matriz_filas(const matriz_t *matriz) { return matriz->filas; }
size_t matriz_columnas(const matriz_t *matriz) { return matriz->columnas; }

void matriz_dimensiones(const matriz_t *matriz, size_t *filas, size_t *columnas) {
    *filas = matriz->filas;
    *columnas = matriz->columnas;
}

float matriz_obtener(const matriz_t *matriz, size_t fila, size_t columna) {
    return matriz->m[fila * matriz->columnas + columna];
}

void matriz_establecer(matriz_t *matriz, size_t fila, size_t columna, float valor) {
    matriz->m[fila * matriz->columnas + columna] = valor;
}

matriz_t *matriz_crear_identidad(size_t n) {
    matriz_t *mat = _matriz_crear(n, n);
    if (!mat) return NULL;
    for (size_t i = 0; i < n; i++)
        mat->m[i * n + i] = 1.0f;
    return mat;
}

matriz_t *matriz_crear_mn(size_t n) {
    matriz_t *mat = matriz_crear_identidad(n);
    if (!mat) return NULL;
    mat->m[(n-1)*n + (n-2)] = -1.0f;
    mat->m[(n-1)*n + (n-1)] = 0.0f;
    return mat;
}

matriz_t *matriz_crear_mz(double ac) {
    matriz_t *mat = matriz_crear_identidad(4);
    if (!mat) return NULL;
    float c = cos(ac), s = sin(ac);
    mat->m[0*4+0] = c;  mat->m[0*4+1] = -s;
    mat->m[1*4+0] = s;  mat->m[1*4+1] = c;
    return mat;
}

matriz_t *matriz_crear_my(double ab) {
    matriz_t *mat = matriz_crear_identidad(4);
    if (!mat) return NULL;
    float c = cos(ab), s = sin(ab);
    mat->m[0*4+0] = c;  mat->m[0*4+2] = s;
    mat->m[2*4+0] = -s; mat->m[2*4+2] = c;
    return mat;
}

matriz_t *matriz_crear_mc(double ac) {
    return matriz_crear_mz(ac);
}

matriz_t *matriz_crear_mt(const float vector[3]) {
    matriz_t *mat = matriz_crear_identidad(4);
    if (!mat) return NULL;
    for (size_t i = 0; i < 3; i++)
        mat->m[i*4 + 3] = vector[i];
    return mat;
}

matriz_t *matriz_crear_mper(float d) {
    matriz_t *mat = matriz_crear_identidad(4);
    if (!mat) return NULL;
    mat->m[3*4 + 2] = -1.0f / d;
    mat->m[3*4 + 3] = 0.0f;
    return mat;
}

matriz_t *matriz_multiplicar(const matriz_t *a, const matriz_t *b) {
    size_t fa = a->filas, ca = a->columnas;
    size_t fb = b->filas, cb = b->columnas;
    if (ca != fb) return NULL;

    matriz_t *r = _matriz_crear(fa, cb);
    if (!r) return NULL;

    for (size_t i = 0; i < fa; i++)
        for (size_t j = 0; j < cb; j++)
            for (size_t k = 0; k < ca; k++)
                r->m[i*cb + j] += a->m[i*ca + k] * b->m[k*cb + j];

    return r;
}

bool matriz_agregar_fila(matriz_t *matriz, const float fila[]) {
    size_t cols = matriz->columnas;
    float *nuevo = realloc(matriz->m, (matriz->filas + 1) * cols * sizeof(float));
    if (!nuevo) return false;
    matriz->m = nuevo;
    for (size_t i = 0; i < cols; i++)
        matriz->m[matriz->filas * cols + i] = fila[i];
    matriz->filas++;
    return true;
}

matriz_t *matriz_aplicar(const matriz_t *transform, const matriz_t *puntos) {
    size_t n = matriz_filas(puntos);

    matriz_t *r = _matriz_crear(n, 3);
    if (!r) return NULL;

    for (size_t i = 0; i < n; i++) {
        float pv[4] = {
            matriz_obtener(puntos, i, 0),
            matriz_obtener(puntos, i, 1),
            matriz_obtener(puntos, i, 2),
            1.0f
        };
        float res[4] = {0, 0, 0, 0};
        for (size_t f = 0; f < 4; f++)
            for (size_t c = 0; c < 4; c++)
                res[f] += matriz_obtener(transform, f, c) * pv[c];

        matriz_establecer(r, i, 0, res[0] / res[3]);
        matriz_establecer(r, i, 1, res[1] / res[3]);
        matriz_establecer(r, i, 2, res[3]);
    }

    return r;
}
