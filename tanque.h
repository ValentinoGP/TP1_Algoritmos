#ifndef TANQUE_H
#define TANQUE_H

#include <stdbool.h>
#include <stddef.h>
#include "obstaculo.h"

typedef struct tanque tanque_t;

typedef enum {
    MOV_NINGUNO,
    MOV_ADELANTE,
    MOV_ATRAS,
    MOV_GIRAR_IZQ,
    MOV_GIRAR_DER
} movimiento_e;

tanque_t *tanque_crear(float x, float y, float phi, int vidas);
void      tanque_destruir(tanque_t *t);

float tanque_x(const tanque_t *t);
float tanque_y(const tanque_t *t);
float tanque_phi(const tanque_t *t);
int   tanque_vidas(const tanque_t *t);
float tanque_torreta(const tanque_t *t);
bool  tanque_puede_disparar(const tanque_t *t);
bool  tanque_misil_activo(const tanque_t *t);
float tanque_misil_x(const tanque_t *t);
float tanque_misil_y(const tanque_t *t);
float tanque_misil_phi(const tanque_t *t);

void tanque_girar(tanque_t *t, float delta_phi);
void tanque_mover(tanque_t *t, float delta);
void tanque_girar_torreta(tanque_t *t, float delta);
void tanque_recibir_impacto(tanque_t *t);
bool tanque_disparar(tanque_t *t);
void tanque_actualizar(tanque_t *t, float dt);
void tanque_set_posicion(tanque_t *t, float x, float y);
void tanque_desactivar_misil(tanque_t *t);

void tanque_iniciar_movimiento(tanque_t *t, movimiento_e mov);
movimiento_e tanque_movimiento(const tanque_t *t);

tanque_t *crear_tanque_enemigo(float x, float y, float phi, int vidas,
                                obstaculo_t *obs[], size_t n_obs);

#endif