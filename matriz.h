#ifndef MATRIZ_H
#define MATRIZ_H

#include <stddef.h>
#include <stdbool.h>

typedef struct matriz matriz_t;

matriz_t *_matriz_crear(size_t n, size_t m);
void      matriz_destruir(matriz_t *matriz);

size_t   matriz_filas(const matriz_t *matriz);
size_t   matriz_columnas(const matriz_t *matriz);
void     matriz_dimensiones(const matriz_t *matriz, size_t *filas, size_t *columnas);
float    matriz_obtener(const matriz_t *matriz, size_t fila, size_t columna);
void     matriz_establecer(matriz_t *matriz, size_t fila, size_t columna, float valor);

matriz_t *matriz_crear_identidad(size_t n);
matriz_t *matriz_crear_mn(size_t n);
matriz_t *matriz_crear_mc(double ac);
matriz_t *matriz_crear_my(double ab);
matriz_t *matriz_crear_mz(double ac);
matriz_t *matriz_crear_mt(const float vector[3]);
matriz_t *matriz_crear_mper(float d);
matriz_t *matriz_multiplicar(const matriz_t *a, const matriz_t *b);

bool matriz_agregar_fila(matriz_t *matriz, const float fila[]);

matriz_t *matriz_aplicar(const matriz_t *transform, const matriz_t *puntos);

#endif
