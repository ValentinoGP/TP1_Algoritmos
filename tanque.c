#include "tanque.h"
#include "obstaculo.h"
#include <stdlib.h>
#include <math.h>
#define M_PI 3.14

struct tanque {
    float x, y;
    float phi;
    float torreta;
    int vidas;
    float enfriamiento;
    float tiempo_movimiento;
    movimiento_e mov_actual;
    bool misil_activo;
    float misil_x, misil_y;
    float misil_phi;
    float misil_tiempo;
};

tanque_t *tanque_crear(float x, float y, float phi, int vidas) {
    tanque_t *t = malloc(sizeof(tanque_t));
    if (!t) return NULL;
    t->x = x;
    t->y = y;
    t->phi = phi;
    t->torreta = 0;
    t->vidas = vidas;
    t->enfriamiento = 0;
    t->tiempo_movimiento = 0;
    t->mov_actual = MOV_NINGUNO;
    t->misil_activo = false;
    return t;
}

void tanque_destruir(tanque_t *t) { free(t); }

float tanque_x(const tanque_t *t)              { return t->x; }
float tanque_y(const tanque_t *t)              { return t->y; }
float tanque_phi(const tanque_t *t)            { return t->phi; }
int   tanque_vidas(const tanque_t *t)          { return t->vidas; }
float tanque_torreta(const tanque_t *t)        { return t->torreta; }
bool  tanque_puede_disparar(const tanque_t *t) { return t->enfriamiento <= 0; }
bool  tanque_misil_activo(const tanque_t *t)   { return t->misil_activo; }
float tanque_misil_x(const tanque_t *t)        { return t->misil_x; }
float tanque_misil_y(const tanque_t *t)        { return t->misil_y; }
float tanque_misil_phi(const tanque_t *t)      { return t->misil_phi; }

void tanque_girar(tanque_t *t, float delta_phi) {
    t->phi += delta_phi;
    if (t->phi > M_PI)  t->phi -= 2 * M_PI;
    if (t->phi <= -M_PI) t->phi += 2 * M_PI;
}

void tanque_mover(tanque_t *t, float delta) {
    t->x += delta * cos(t->phi);
    t->y += delta * sin(t->phi);
}

void tanque_girar_torreta(tanque_t *t, float delta) {
    t->torreta += delta;
    if (t->torreta >  1.0f) t->torreta =  1.0f;
    if (t->torreta < -1.0f) t->torreta = -1.0f;
}

void tanque_recibir_impacto(tanque_t *t) { t->vidas--; }

bool tanque_disparar(tanque_t *t) {
    if (t->enfriamiento > 0 || t->misil_activo) return false;
    t->misil_activo = true;
    t->misil_x = t->x;
    t->misil_y = t->y;
    t->misil_phi = t->phi + t->torreta;
    t->misil_tiempo = 2.0f;
    t->enfriamiento = 2.0f;
    return true;
}

void tanque_iniciar_movimiento(tanque_t *t, movimiento_e mov) {
    t->mov_actual = mov;
    t->tiempo_movimiento = 0.5f;
}

movimiento_e tanque_movimiento(const tanque_t *t) {
    return t->mov_actual;
}

void tanque_actualizar(tanque_t *t, float dt) {
    if (t->enfriamiento > 0) t->enfriamiento -= dt;

    if (t->tiempo_movimiento > 0) {
        t->tiempo_movimiento -= dt;
        switch (t->mov_actual) {
            case MOV_ADELANTE:
                t->x += 7.0f * cos(t->phi) * dt;
                t->y += 7.0f * sin(t->phi) * dt;
                break;
            case MOV_ATRAS:
                t->x -= 7.0f * cos(t->phi) * dt;
                t->y -= 7.0f * sin(t->phi) * dt;
                break;
            case MOV_GIRAR_IZQ:
                t->phi -= 0.36f * dt;
                if (t->phi <= -M_PI) t->phi += 2 * M_PI;
                break;
            case MOV_GIRAR_DER:
                t->phi += 0.36f * dt;
                if (t->phi > M_PI) t->phi -= 2 * M_PI;
                break;
            default: break;
        }
        if (t->tiempo_movimiento <= 0)
            t->mov_actual = MOV_NINGUNO;
    }

    if (t->misil_activo) {
        t->misil_x += 24.0f * cos(t->misil_phi) * dt;
        t->misil_y += 24.0f * sin(t->misil_phi) * dt;
        t->misil_tiempo -= dt;
        if (t->misil_tiempo <= 0) t->misil_activo = false;
    }
}

tanque_t *crear_tanque_enemigo(float x, float y, float phi, int vidas,
                                 obstaculo_t *obs[], size_t n_obs) {
    for (size_t i = 0; i < n_obs; i++) {
        float dx = x - obstaculo_x(obs[i]);
        float dy = y - obstaculo_y(obs[i]);
        if (sqrtf(dx*dx + dy*dy) < 5.0f) return NULL;
    }
    return tanque_crear(x, y, phi, vidas);
}
