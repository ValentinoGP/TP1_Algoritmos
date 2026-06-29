#include <SDL2/SDL.h>
#include "modelo.h"
#include "obstaculo.h"
#include "tanque.h"
#include "stl.h"
#include "matriz.h"
#include "pila.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VENTANA_ANCHO 1024
#define VENTANA_ALTO 768
#define JUEGO_FPS 24
#define MUNDO 150.0f

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
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

    srand(SDL_GetTicks());

    // find models by name
    modelo_t *modelo_tanque = NULL, *modelo_torreta = NULL, *modelo_radar = NULL;
    modelo_t *modelo_misil = NULL, *modelo_horizonte = NULL;
    modelo_t *modelo_montanas = NULL, *modelo_luna = NULL;
    modelo_t *obs_models[6] = {NULL};
    int modelos_cargados = 0;
    for (struct nodo_m *n = lista_modelos; n; n = n->sig) {
        modelos_cargados++;
        const char *name = modelo_nombre(n->modelo);
        if      (strcmp(name, "TANQUE") == 0)    modelo_tanque = n->modelo;
        else if (strcmp(name, "TORRETA") == 0)   modelo_torreta = n->modelo;
        else if (strcmp(name, "RADAR") == 0)     modelo_radar = n->modelo;
        else if (strcmp(name, "MISIL") == 0)     modelo_misil = n->modelo;
        else if (strcmp(name, "HORIZONTE") == 0) modelo_horizonte = n->modelo;
        else if (strcmp(name, "MONTANAS") == 0)  modelo_montanas = n->modelo;
        else if (strcmp(name, "LUNA") == 0)      modelo_luna = n->modelo;
        else if (strcmp(name, "CUBO1") == 0)     obs_models[0] = n->modelo;
        else if (strcmp(name, "CUBO2") == 0)     obs_models[1] = n->modelo;
        else if (strcmp(name, "CUBO3") == 0)     obs_models[2] = n->modelo;
        else if (strcmp(name, "PIRAMIDE1") == 0) obs_models[3] = n->modelo;
        else if (strcmp(name, "PIRAMIDE2") == 0) obs_models[4] = n->modelo;
        else if (strcmp(name, "PIRAMIDE3") == 0) obs_models[5] = n->modelo;
    }
    if (modelos_cargados == 0) fprintf(stderr, "ERROR: no se cargaron modelos desde modelos.stl\n");

    // 50 random obstacles in [-150, 150]
    size_t num_obs = 50;
    obstaculo_t *obstaculos[num_obs];
    int obs_available = 0;
    for (int k = 0; k < 6; k++) if (obs_models[k]) obs_available = 1;
    if (!obs_available) {
        num_obs = 0;
        fprintf(stderr, "ERROR: no hay modelos de obstáculos (CUBO1-3, PIRAMIDE1-3)\n");
    }
    for (size_t i = 0; i < num_obs; ) {
        int idx = rand() % 6;
        if (!obs_models[idx]) continue;
        float ox = (rand() % (int)(MUNDO * 200)) / 100.0f - MUNDO;
        float oy = (rand() % (int)(MUNDO * 200)) / 100.0f - MUNDO;
        float op = (rand() % 628) / 100.0f - M_PI;
        obstaculos[i] = obstaculo_crear(ox, oy, op, obs_models[idx]);
        i++;
    }

    // player at origin facing +Y (phi = pi/2), 4 lives
    tanque_t *jugador = tanque_crear(0, 0, M_PI / 2, 4);

    // enemy 50 units from player, not on obstacle
    tanque_t *enemigo = NULL;
    while (!enemigo) {
        float ang = (rand() % 628) / 100.0f;
        float ex = 50.0f * cos(ang);
        float ey = 50.0f * sin(ang);
        enemigo = crear_tanque_enemigo(ex, ey, M_PI / 2, 3, obstaculos, num_obs);
    }

    unsigned int prev_ticks = SDL_GetTicks();
    float angx = 0, angz = 0;

    pila_t *stack = pila_crear();
    matriz_t *ident = matriz_crear_identidad(4);
    pila_apilar(stack, ident);

    // END código del alumno

    unsigned int ticks = SDL_GetTicks();
    int done = 0;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
                break;
            }

            // BEGIN código del alumno
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        tanque_iniciar_movimiento(jugador, MOV_ADELANTE);
                        break;
                    case SDLK_DOWN:
                        tanque_iniciar_movimiento(jugador, MOV_ATRAS);
                        break;
                    case SDLK_RIGHT:
                        tanque_iniciar_movimiento(jugador, MOV_GIRAR_DER);
                        break;
                    case SDLK_LEFT:
                        tanque_iniciar_movimiento(jugador, MOV_GIRAR_IZQ);
                        break;
                    case ' ':
                        if (tanque_puede_disparar(jugador))
                            tanque_disparar(jugador);
                        break;
                }
            }
            // END código del alumno
        }
        if (done) break;

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        // BEGIN código del alumno
        float dt = (SDL_GetTicks() - prev_ticks) / 1000.0f;
        if (dt > 0.1f) dt = 0.1f;
        prev_ticks = SDL_GetTicks();

        // head bob
        movimiento_e mov = tanque_movimiento(jugador);
        if (mov != MOV_NINGUNO) {
            if (mov == MOV_GIRAR_IZQ || mov == MOV_GIRAR_DER)
                angz += ((rand() % 2001) - 1000) / 100000.0f;
            else
                angx += ((rand() % 2001) - 1000) / 100000.0f;
        }

        tanque_actualizar(jugador, dt);
        tanque_actualizar(enemigo, dt);

        // build camera matrix: MPER × Mz(π/2+angz) × My(π/2-angx) × Mz(-phi) × Mt(-x, -y, -3)
        float px = tanque_x(jugador), py = tanque_y(jugador), pp = tanque_phi(jugador);

        matriz_t *cam = matriz_crear_identidad(4);
        matriz_t *tmp;

        matriz_t *mper = matriz_crear_mper(4);
        tmp = matriz_multiplicar(cam, mper);
        matriz_destruir(cam); cam = tmp;
        matriz_destruir(mper);

        matriz_t *mz1 = matriz_crear_mz(M_PI / 2 + angz);
        tmp = matriz_multiplicar(cam, mz1);
        matriz_destruir(cam); cam = tmp;
        matriz_destruir(mz1);

        float angx_rad = angx > 0.01f ? angx : 0;
        matriz_t *my1 = matriz_crear_my(M_PI / 2 - angx_rad);
        tmp = matriz_multiplicar(cam, my1);
        matriz_destruir(cam); cam = tmp;
        matriz_destruir(my1);

        matriz_t *mz2 = matriz_crear_mz(-pp);
        tmp = matriz_multiplicar(cam, mz2);
        matriz_destruir(cam); cam = tmp;
        matriz_destruir(mz2);

        float vec[3] = {-px, -py, -3.0f};
        matriz_t *mt = matriz_crear_mt(vec);
        tmp = matriz_multiplicar(cam, mt);
        matriz_destruir(cam); cam = tmp;
        matriz_destruir(mt);

        pila_apilar(stack, cam);

        // helper: render a modelo at a given world position+rotation
        // assumes camera matrix is at stack top
        #define RENDER_MODELO(m, x, y, z, rot) do { \
            if (!(m)) break; \
            float tv[3] = {(x), (y), (z)}; \
            matriz_t *_mt = matriz_crear_mt(tv); \
            matriz_t *_mr = matriz_crear_mz(rot); \
            matriz_t *_obj = matriz_multiplicar(_mt, _mr); \
            matriz_t *_cm = pila_tope(stack); \
            matriz_t *_final = matriz_multiplicar(_cm, _obj); \
            pila_apilar(stack, _final); \
            \
            const float *_c = modelo_coords(m); \
            const size_t *_l = modelo_lineas(m); \
            size_t _nv = modelo_ncoords(m); \
            size_t _nl = modelo_nlineas(m); \
            \
            matriz_t *_pts = _matriz_crear(_nv, 3); \
            for (size_t _k = 0; _k < _nv; _k++) { \
                matriz_establecer(_pts, _k, 0, _c[3*_k]); \
                matriz_establecer(_pts, _k, 1, _c[3*_k+1]); \
                matriz_establecer(_pts, _k, 2, _c[3*_k+2]); \
            } \
            \
            matriz_t *_proj = matriz_aplicar(pila_tope(stack), _pts); \
            \
            for (size_t _j = 0; _j < 2 * _nl; _j += 2) { \
                size_t _i0 = _l[_j], _i1 = _l[_j+1]; \
                float _d0 = matriz_obtener(_proj, _i0, 2); \
                float _d1 = matriz_obtener(_proj, _i1, 2); \
                if (_d0 >= 1.0f && _d1 >= 1.0f) { \
                    float _x0 = matriz_obtener(_proj, _i0, 0); \
                    float _y0 = matriz_obtener(_proj, _i0, 1); \
                    float _x1 = matriz_obtener(_proj, _i1, 0); \
                    float _y1 = matriz_obtener(_proj, _i1, 1); \
                    int _sx0 = (int)(_x0 * VENTANA_ALTO/2 + VENTANA_ANCHO/2); \
                    int _sy0 = (int)(VENTANA_ALTO/2 - _y0 * VENTANA_ALTO/2); \
                    int _sx1 = (int)(_x1 * VENTANA_ALTO/2 + VENTANA_ANCHO/2); \
                    int _sy1 = (int)(VENTANA_ALTO/2 - _y1 * VENTANA_ALTO/2); \
                    SDL_RenderDrawLine(renderer, _sx0, _sy0, _sx1, _sy1); \
                } \
            } \
            \
            matriz_destruir(_proj); \
            matriz_destruir(_pts); \
            matriz_destruir(pila_desapilar(stack)); \
            matriz_destruir(_obj); \
            matriz_destruir(_mr); \
            matriz_destruir(_mt); \
        } while(0)

        // obstacles (white)
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
        for (size_t i = 0; i < num_obs; i++)
            RENDER_MODELO(obstaculo_modelo(obstaculos[i]),
                          obstaculo_x(obstaculos[i]),
                          obstaculo_y(obstaculos[i]),
                          0.0f,
                          obstaculo_phi(obstaculos[i]));

        // enemy tank (red): hull + turret + radar
        if (modelo_tanque && modelo_torreta) {
            float ex = tanque_x(enemigo), ey = tanque_y(enemigo);
            float ep = tanque_phi(enemigo), et = tanque_torreta(enemigo);

            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
            RENDER_MODELO(modelo_tanque, ex, ey, 0.0f, ep);

            SDL_SetRenderDrawColor(renderer, 0xFF, 0x80, 0x00, 0x00);
            RENDER_MODELO(modelo_torreta, ex, ey, 3.0f, ep + et);

            if (modelo_radar) {
                float radar_x = ex + (-1.5f)*cosf(ep+et) - 0*sinf(ep+et);
                float radar_y = ey + (-1.5f)*sinf(ep+et) + 0*cosf(ep+et);
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0x00);
                RENDER_MODELO(modelo_radar, radar_x, radar_y, 3.5f, ep + et);
            }
        }

        // missiles (if active)
        tanque_t *tanks[2] = {jugador, enemigo};
        Uint8 mcols[2][3] = {{0x00, 0xFF, 0x00}, {0xFF, 0x00, 0x00}};
        for (int ti = 0; ti < 2; ti++) {
            if (modelo_misil && tanque_misil_activo(tanks[ti])) {
                SDL_SetRenderDrawColor(renderer, mcols[ti][0], mcols[ti][1], mcols[ti][2], 0x00);
                RENDER_MODELO(modelo_misil,
                              tanque_misil_x(tanks[ti]),
                              tanque_misil_y(tanks[ti]),
                              0.0f,
                              tanque_misil_phi(tanks[ti]));
            }
        }

        // background (drawn at origin with no camera offset)
        if (modelo_horizonte) {
            SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0x00);
            RENDER_MODELO(modelo_horizonte, 0, 0, 0, 0);
        }
        if (modelo_montanas) {
            SDL_SetRenderDrawColor(renderer, 0x40, 0x40, 0x40, 0x00);
            RENDER_MODELO(modelo_montanas, 0, 0, 0, 0);
        }
        if (modelo_luna) {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xCC, 0x00);
            RENDER_MODELO(modelo_luna, 0, 0, 0, 0);
        }

        #undef RENDER_MODELO

        // pop camera matrix
        matriz_destruir(pila_desapilar(stack));

        // crosshair
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
        SDL_RenderDrawLine(renderer, VENTANA_ANCHO/2 - 8, VENTANA_ALTO/2,
                           VENTANA_ANCHO/2 + 8, VENTANA_ALTO/2);
        SDL_RenderDrawLine(renderer, VENTANA_ANCHO/2, VENTANA_ALTO/2 - 8,
                           VENTANA_ANCHO/2, VENTANA_ALTO/2 + 8);
        // END código del alumno

        SDL_RenderPresent(renderer);
        ticks = SDL_GetTicks() - ticks;
        if (dormir) {
            SDL_Delay(dormir);
            dormir = 0;
        } else if (ticks < 1000 / JUEGO_FPS)
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
    for (size_t i = 0; i < num_obs; i++)
        obstaculo_destruir(obstaculos[i]);
    tanque_destruir(jugador);
    tanque_destruir(enemigo);
    pila_destruir(stack);
    // END código del alumno

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
