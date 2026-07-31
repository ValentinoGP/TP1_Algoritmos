#include <SDL2/SDL.h>
#include <stdio.h>
#include "juego.h"
#include "dibujo.h"

#define VENTANA_ANCHO 1024
#define VENTANA_ALTO 768
#define FPS_JUEGO 24

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *ventana;
    SDL_Renderer *renderizador;

    SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &ventana, &renderizador);
    SDL_SetWindowTitle(ventana, "Battle Zone");

    juego_t *juego = juego_crear();
    dibujo_t *dibujo = dibujo_crear(renderizador, juego);

    unsigned int tick_anterior = SDL_GetTicks();
    int terminado = 0;

    while (!terminado) {
        SDL_Event evento;
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                terminado = 1;
                break;
            }
            juego_tratar_evento(juego, &evento);
        }
        if (terminado) break;

        SDL_SetRenderDrawColor(renderizador, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderizador);

        juego_actualizar(juego);

        if (!juego_terminado(juego))
            dibujo_dibujar_mundo(dibujo, juego);
        dibujo_dibujar_hud(dibujo, juego);

        SDL_RenderPresent(renderizador);

        unsigned int tiempo_cuadro = SDL_GetTicks() - tick_anterior;
        if (tiempo_cuadro < 1000 / FPS_JUEGO)
            SDL_Delay(1000 / FPS_JUEGO - tiempo_cuadro);
        else
            printf("Perdiendo cuadros\n");
        tick_anterior = SDL_GetTicks();
    }

    dibujo_destruir(dibujo);
    juego_destruir(juego);

    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);

    SDL_Quit();
    return 0;
}
