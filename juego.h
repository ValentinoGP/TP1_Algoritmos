#ifndef JUEGO_H
#define JUEGO_H

#include <stdbool.h>
#include <stddef.h>
#include <SDL2/SDL.h>
#include "modelo.h"
#include "tanque.h"
#include "obstaculo.h"
#include "pila.h"

typedef struct juego juego_t;

juego_t *juego_crear(SDL_Renderer *renderer);
void      juego_destruir(juego_t *j);

bool juego_tratar_evento(juego_t *j, const SDL_Event *event);
void juego_actualizar(juego_t *j);

bool   juego_terminado(const juego_t *j);
int    juego_puntaje(const juego_t *j);
float  juego_hit_timer(const juego_t *j);
float  juego_resto_timer(const juego_t *j);
float  juego_resto_x(const juego_t *j);
float  juego_resto_y(const juego_t *j);
float  juego_angx(const juego_t *j);
float  juego_angz(const juego_t *j);

float  juego_jugador_x(const juego_t *j);
float  juego_jugador_y(const juego_t *j);
float  juego_jugador_phi(const juego_t *j);
int    juego_jugador_vidas(const juego_t *j);

bool   juego_enemigo_existe(const juego_t *j);
float  juego_enemigo_x(const juego_t *j);
float  juego_enemigo_y(const juego_t *j);
float  juego_enemigo_phi(const juego_t *j);
float  juego_enemigo_torreta(const juego_t *j);

bool   juego_misil_jugador_activo(const juego_t *j);
float  juego_misil_jugador_x(const juego_t *j);
float  juego_misil_jugador_y(const juego_t *j);
float  juego_misil_jugador_phi(const juego_t *j);

bool   juego_misil_enemigo_activo(const juego_t *j);
float  juego_misil_enemigo_x(const juego_t *j);
float  juego_misil_enemigo_y(const juego_t *j);
float  juego_misil_enemigo_phi(const juego_t *j);

size_t juego_num_obstaculos(const juego_t *j);
const obstaculo_t *juego_obstaculo(const juego_t *j, size_t i);

const modelo_t *juego_modelo(const juego_t *j, const char *nombre);
pila_t *juego_stack(const juego_t *j);

#endif
