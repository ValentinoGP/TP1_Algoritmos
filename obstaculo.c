#include "obstaculo.h"
#include <stdlib.h>
#include <math.h>

struct obstaculo {
    float x, y;
    float phi;
    const modelo_t *modelo;
};

obstaculo_t *obstaculo_crear(float x, float y, float phi, const modelo_t *modelo) {
    obstaculo_t *o = malloc(sizeof(obstaculo_t));
    if (o == NULL) return NULL;
    o->x = x;
    o->y = y;
    o->phi = phi;
    o->modelo = modelo;
    return o;
}

void obstaculo_destruir(obstaculo_t *o) {
    free(o);
}

float obstaculo_x(const obstaculo_t *o) { return o->x; }
float obstaculo_y(const obstaculo_t *o) { return o->y; }
float obstaculo_phi(const obstaculo_t *o) { return o->phi; }
const modelo_t *obstaculo_modelo(const obstaculo_t *o) { return o->modelo; }