#include <SDL2/SDL.h>
#include "modelo.h"
#include "obstaculo.h"
#include "stl.h"
#include "tanque.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define VENTANA_ANCHO 1024
#define VENTANA_ALTO 768

#define JUEGO_FPS 24

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Battle Zone");

    int dormir = 0;

    // BEGIN código del alumno
    struct nodo_m { struct nodo_m *sig; modelo_t *modelo; };
    struct nodo_m *lista_modelos = NULL;

    FILE *f = fopen("modelos.stl", "rb");
    if (f) {
        unidades_t unidades;
        size_t maxlong;
        leer_encabezado_stl(f);
        leer_formato_STL(f, &unidades, &maxlong);
        while (1) {
            float *coords = NULL;
            size_t ncoords = 0;
            size_t *lineas = NULL;
            size_t nlineas = 0;
            char etiqueta[maxlong + 1];
            if (!leer_modelo_3d(f, maxlong, etiqueta, &ncoords, &coords, &nlineas, &lineas))
                break;
            etiqueta[maxlong] = '\0';
            modelo_t *m = modelo_crear(etiqueta, coords, ncoords, lineas, nlineas);
            free(coords);
            free(lineas);
            if (m) {
                struct nodo_m *n = malloc(sizeof(struct nodo_m));
                if (n) {
                    n->modelo = m;
                    n->sig = lista_modelos;
                    lista_modelos = n;
                }
            }
        }
        fclose(f);
    }

    unsigned char color[3] = {0xff, 0, 0}, aux;
    int x = VENTANA_ANCHO / 2;
    int y = VENTANA_ALTO / 2;
    // END código del alumno

    unsigned int ticks = SDL_GetTicks();
    while(1) {
        if(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;

            // BEGIN código del alumno
            if (event.type == SDL_KEYDOWN) {
                // Se apretó una tecla
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        y -= 10;
                        break;
                    case SDLK_DOWN:
                        y += 10;
                        break;
                    case SDLK_RIGHT:
                        x += 10;
                        break;
                    case SDLK_LEFT:
                        x -= 10;
                        break;
                    case ' ':
                        aux = color[0];
                        color[0] = color[1];
                        color[1] = color[2];
                        color[2] = aux;
                        break;
                }
            }
            // END código del alumno

            continue;
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);

        // BEGIN código del alumno
        SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 0x00);
        SDL_RenderDrawLine(renderer, 0, 0, x, y);
        // END código del alumno

        SDL_RenderPresent(renderer);
        ticks = SDL_GetTicks() - ticks;
        if(dormir) {
            SDL_Delay(dormir);
            dormir = 0;
        }
        else if(ticks < 1000 / JUEGO_FPS)
            SDL_Delay(1000 / JUEGO_FPS - ticks);
        else
            printf("Perdiendo cuadros\n");
        ticks = SDL_GetTicks();
    }

    // BEGIN código del alumno
    {
        struct nodo_m *n = lista_modelos;
        while (n) {
            struct nodo_m *sig = n->sig;
            modelo_destruir(n->modelo);
            free(n);
            n = sig;
        }
    }
    // END código del alumno

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
