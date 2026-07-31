#ifndef JUEGO_H
#define JUEGO_H

#include <stdbool.h>
#include <stddef.h>
#include <SDL2/SDL.h>
#include "modelo.h"
#include "tanque.h"
#include "obstaculo.h"

typedef struct juego juego_t;

juego_t *juego_crear(void);
void      juego_destruir(juego_t *juego);

bool juego_tratar_evento(juego_t *juego, const SDL_Event *evento);
void juego_actualizar(juego_t *juego);

bool   juego_terminado(const juego_t *juego);
int    juego_puntaje(const juego_t *juego);
float  juego_tiempo_impacto(const juego_t *juego);
float  juego_tiempo_resto(const juego_t *juego);
float  juego_resto_x(const juego_t *juego);
float  juego_resto_y(const juego_t *juego);
float  juego_angx(const juego_t *juego);
float  juego_angz(const juego_t *juego);

float  juego_jugador_x(const juego_t *juego);
float  juego_jugador_y(const juego_t *juego);
float  juego_jugador_phi(const juego_t *juego);
int    juego_jugador_vidas(const juego_t *juego);

bool   juego_enemigo_existe(const juego_t *juego);
float  juego_enemigo_x(const juego_t *juego);
float  juego_enemigo_y(const juego_t *juego);
float  juego_enemigo_phi(const juego_t *juego);
float  juego_enemigo_torreta(const juego_t *juego);

bool   juego_misil_jugador_activo(const juego_t *juego);
float  juego_misil_jugador_x(const juego_t *juego);
float  juego_misil_jugador_y(const juego_t *juego);
float  juego_misil_jugador_phi(const juego_t *juego);

bool   juego_misil_enemigo_activo(const juego_t *juego);
float  juego_misil_enemigo_x(const juego_t *juego);
float  juego_misil_enemigo_y(const juego_t *juego);
float  juego_misil_enemigo_phi(const juego_t *juego);

size_t juego_cantidad_obstaculos(const juego_t *juego);
const obstaculo_t *juego_obstaculo(const juego_t *juego, size_t i);

const modelo_t *juego_modelo(const juego_t *juego, const char *nombre);

#endif
