#ifndef DIBUJO_H
#define DIBUJO_H

#include <SDL2/SDL.h>
#include "juego.h"

typedef struct dibujo dibujo_t;

dibujo_t *dibujo_crear(SDL_Renderer *renderizador, const juego_t *juego);
void      dibujo_destruir(dibujo_t *dibujo);

void dibujo_dibujar_mundo(dibujo_t *dibujo, const juego_t *juego);
void dibujo_dibujar_hud(dibujo_t *dibujo, const juego_t *juego);

#endif
