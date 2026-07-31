#include "misil.h"
#include <stdlib.h>
#include <math.h>

#define VELOCIDAD_MISIL 24.0f
#define TIEMPO_VIDA_MISIL 2.0f

struct misil {
    float x, y;
    float phi;
    bool activo;
    float tiempo;
};

misil_t *misil_crear(void) {
    misil_t *m = malloc(sizeof(misil_t));
    if (!m) return NULL;
    m->x = 0;
    m->y = 0;
    m->phi = 0;
    m->activo = false;
    m->tiempo = 0;
    return m;
}

void misil_destruir(misil_t *m) { free(m); }

void misil_lanzar(misil_t *m, float x, float y, float phi) {
    m->x = x;
    m->y = y;
    m->phi = phi;
    m->activo = true;
    m->tiempo = TIEMPO_VIDA_MISIL;
}

void misil_desactivar(misil_t *m) { m->activo = false; }

void misil_actualizar(misil_t *m, float dt) {
    if (!m->activo) return;
    m->x += VELOCIDAD_MISIL * cosf(m->phi) * dt;
    m->y += VELOCIDAD_MISIL * sinf(m->phi) * dt;
    m->tiempo -= dt;
    if (m->tiempo <= 0) m->activo = false;
}

bool  misil_activo(const misil_t *m) { return m->activo; }
float misil_x(const misil_t *m)       { return m->x; }
float misil_y(const misil_t *m)       { return m->y; }
float misil_phi(const misil_t *m)     { return m->phi; }
