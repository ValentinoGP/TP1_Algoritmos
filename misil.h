#ifndef MISIL_H
#define MISIL_H

#include <stdbool.h>

typedef struct misil misil_t;

misil_t *misil_crear(void);
void     misil_destruir(misil_t *m);

void  misil_lanzar(misil_t *m, float x, float y, float phi);
void  misil_desactivar(misil_t *m);
void  misil_actualizar(misil_t *m, float dt);

bool  misil_activo(const misil_t *m);
float misil_x(const misil_t *m);
float misil_y(const misil_t *m);
float misil_phi(const misil_t *m);

#endif
