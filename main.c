#include <SDL2/SDL.h>
#include "modelo.h"
#include "obstaculo.h"
#include "tanque.h"
#include "stl.h"
#include "matriz.h"
#include "pila.h"
#include "lista.h"
#include "cola.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VENTANA_ANCHO 1024
#define VENTANA_ALTO 768
#define JUEGO_FPS 24
#define MUNDO 150.0f
#define M_PI 3.14

static void renderizar_modelo(SDL_Renderer *r, pila_t *p, const modelo_t *modelo,
                               float x, float y, float z, float rot,
                               size_t max_lineas) {
    if (!modelo) return;

    float vec[3] = {x, y, z};
    matriz_t *mt = matriz_crear_mt(vec);
    matriz_t *mr = matriz_crear_mz(rot);
    matriz_t *obj = matriz_multiplicar(mt, mr);
    matriz_destruir(mt);
    matriz_destruir(mr);

    matriz_t *cam = pila_ver_tope(p);
    matriz_t *final = matriz_multiplicar(cam, obj);
    pila_apilar(p, final);
    matriz_destruir(obj);

    const float *coords = modelo_coords(modelo);
    const size_t *lineas = modelo_lineas(modelo);
    size_t ncoords = modelo_ncoords(modelo);
    size_t nlineas = modelo_nlineas(modelo);
    if (max_lineas == 0 || max_lineas > nlineas) max_lineas = nlineas;

    matriz_t *pts = _matriz_crear(ncoords, 3);
    for (size_t i = 0; i < ncoords; i++) {
        matriz_establecer(pts, i, 0, coords[3*i]);
        matriz_establecer(pts, i, 1, coords[3*i+1]);
        matriz_establecer(pts, i, 2, coords[3*i+2]);
    }

    matriz_t *proj = matriz_aplicar(pila_ver_tope(p), pts);
    matriz_destruir(pts);

    for (size_t i = 0; i < 2 * max_lineas; i += 2) {
        size_t i0 = lineas[i], i1 = lineas[i+1];
        float d0 = matriz_obtener(proj, i0, 2);
        float d1 = matriz_obtener(proj, i1, 2);
        if (d0 >= 1.0f && d1 >= 1.0f) {
            float x0 = matriz_obtener(proj, i0, 0);
            float y0 = matriz_obtener(proj, i0, 1);
            float x1 = matriz_obtener(proj, i1, 0);
            float y1 = matriz_obtener(proj, i1, 1);
            int sx0 = (int)(x0 * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
            int sy0 = (int)(VENTANA_ALTO/2 - y0 * VENTANA_ALTO/2);
            int sx1 = (int)(x1 * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
            int sy1 = (int)(VENTANA_ALTO/2 - y1 * VENTANA_ALTO/2);
            SDL_RenderDrawLine(r, sx0, sy0, sx1, sy1);
        }
    }

    matriz_destruir(proj);
    matriz_destruir(pila_desapilar(p));
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Battle Zone");

    int dormir = 0;

    // BEGIN código del alumno

    lista_t *lista_modelos = lista_crear();
    cola_t *cola_modelos = cola_crear();

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

            if (m)
                cola_encolar(cola_modelos, m);
        }
        fclose(f);
    }

    srand(SDL_GetTicks());

    modelo_t *modelo_tanque = NULL, *modelo_torreta = NULL, *modelo_radar = NULL;
    modelo_t *modelo_misil = NULL, *modelo_horizonte = NULL;
    modelo_t *modelo_montanas = NULL, *modelo_luna = NULL;
    modelo_t *obs_models[6] = {NULL};
    modelo_t *modelo_letras[26] = {NULL};
    modelo_t *modelo_numeros[10] = {NULL};
    modelo_t *modelo_star = NULL, *modelo_hash = NULL;
    modelo_t *modelo_resto1 = NULL, *modelo_resto2 = NULL;
    int puntaje = 0;
    bool game_over = false;
    float hit_timer = 0, resto_timer = 0;
    float resto_x = 0, resto_y = 0;

    while (!cola_esta_vacia(cola_modelos)) {
        modelo_t *m = cola_desencolar(cola_modelos);
        lista_insertar_ultimo(lista_modelos, m);
    }
    cola_destruir(cola_modelos, NULL);

    lista_iter_t *it = lista_iter_crear(lista_modelos);
    while (!lista_iter_al_final(it)) {
        modelo_t *m = lista_iter_ver_actual(it);
        const char *name = modelo_nombre(m);

        if      (strcmp(name, "TANQUE") == 0)    modelo_tanque = m;
        else if (strcmp(name, "TORRETA") == 0)   modelo_torreta = m;
        else if (strcmp(name, "RADAR") == 0)     modelo_radar = m;
        else if (strcmp(name, "MISIL") == 0)     modelo_misil = m;
        else if (strcmp(name, "HORIZONTE") == 0) modelo_horizonte = m;
        else if (strcmp(name, "MONTANA") == 0)   modelo_montanas = m;
        else if (strcmp(name, "LUNA") == 0)      modelo_luna = m;
        else if (strcmp(name, "CUBO1") == 0)     obs_models[0] = m;
        else if (strcmp(name, "CUBO2") == 0)     obs_models[1] = m;
        else if (strcmp(name, "CUBO3") == 0)     obs_models[2] = m;
        else if (strcmp(name, "PIRAMIDE1") == 0) obs_models[3] = m;
        else if (strcmp(name, "PIRAMIDE2") == 0) obs_models[4] = m;
        else if (strcmp(name, "PIRAMIDE3") == 0) obs_models[5] = m;
        else if (strlen(name) == 1 && name[0] >= 'A' && name[0] <= 'Z')
            modelo_letras[name[0] - 'A'] = m;
        else if (strlen(name) == 1 && name[0] >= '0' && name[0] <= '9')
            modelo_numeros[name[0] - '0'] = m;
        else if (strcmp(name, "*") == 0)     modelo_star = m;
        else if (strcmp(name, "#") == 0)     modelo_hash = m;
        else if (strcmp(name, "RESTO1") == 0) modelo_resto1 = m;
        else if (strcmp(name, "RESTO2") == 0) modelo_resto2 = m;

        lista_iter_avanzar(it);
    }
    lista_iter_destruir(it);

    if (lista_largo(lista_modelos) == 0)
        fprintf(stderr, "ERROR: no se cargaron modelos desde modelos.stl\n");

    size_t num_obs = 50;
    obstaculo_t *obstaculos[num_obs];
    int obs_available = 0;
    for (int k = 0; k < 6; k++) {
        if (obs_models[k])
            obs_available = 1;
    }
    if (!obs_available) {
        num_obs = 0;
        fprintf(stderr, "ERROR: no hay modelos de obstaculos (CUBO1-3, PIRAMIDE1-3)\n");
    }
    for (size_t i = 0; i < num_obs; ) {
        int idx = rand() % 6;
        if (!obs_models[idx]) continue;
        float ox = (rand() % (int)(MUNDO * 200 + 1)) / 100.0f - MUNDO;
        float oy = (rand() % (int)(MUNDO * 200 + 1)) / 100.0f - MUNDO;
        float op = (rand() % 628) / 100.0f - M_PI;
        obstaculos[i] = obstaculo_crear(ox, oy, op, obs_models[idx]);
        i++;
    }

    tanque_t *jugador = tanque_crear(0, 0, M_PI / 2, 4);

    tanque_t *enemigo = NULL;
    while (!enemigo) {
        float ang = (rand() % 628) / 100.0f;
        float dist = 50.0f;
        float ex = tanque_x(jugador) + dist * cos(ang);
        float ey = tanque_y(jugador) + dist * sin(ang);
        enemigo = crear_tanque_enemigo(ex, ey, M_PI / 2, 1, obstaculos, num_obs);
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
                        tanque_iniciar_movimiento(jugador, MOV_GIRAR_IZQ);
                        break;
                    case SDLK_LEFT:
                        tanque_iniciar_movimiento(jugador, MOV_GIRAR_DER);
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

        if (hit_timer > 0) hit_timer -= dt;
        if (resto_timer > 0) resto_timer -= dt;
        if (tanque_vidas(jugador) <= 0) game_over = true;

        if (!game_over) {

            movimiento_e mov = tanque_movimiento(jugador);
            if (mov != MOV_NINGUNO) {
                if (mov == MOV_GIRAR_IZQ || mov == MOV_GIRAR_DER)
                    angz += ((rand() % 2001) - 1000) / 100000.0f;
                else
                    angx += ((rand() % 2001) - 1000) / 100000.0f;
            }
            angx *= 0.92f;
            angz *= 0.92f;

            float old_px = tanque_x(jugador), old_py = tanque_y(jugador);
            float old_ex = tanque_x(enemigo), old_ey = tanque_y(enemigo);
            tanque_actualizar(jugador, dt);
            tanque_actualizar(enemigo, dt);

            float px = tanque_x(jugador), py = tanque_y(jugador);
            int choco_jugador = 0;
            for (size_t i = 0; i < num_obs && !choco_jugador; i++) {
                float dx = px - obstaculo_x(obstaculos[i]);
                float dy = py - obstaculo_y(obstaculos[i]);
                if (dx*dx + dy*dy < 36.0f) choco_jugador = 1;
            }
            if (!choco_jugador) {
                float dx = px - old_ex;
                float dy = py - old_ey;
                if (dx*dx + dy*dy < 36.0f) choco_jugador = 1;
            }
            if (choco_jugador)
                tanque_set_posicion(jugador, old_px, old_py);

            float ex = tanque_x(enemigo), ey = tanque_y(enemigo);
            int choco_enemigo = 0;
            for (size_t i = 0; i < num_obs && !choco_enemigo; i++) {
                float dx = ex - obstaculo_x(obstaculos[i]);
                float dy = ey - obstaculo_y(obstaculos[i]);
                if (dx*dx + dy*dy < 36.0f) choco_enemigo = 1;
            }
            if (!choco_enemigo) {
                float dx = ex - tanque_x(jugador);
                float dy = ey - tanque_y(jugador);
                if (dx*dx + dy*dy < 36.0f) choco_enemigo = 1;
            }
            if (choco_enemigo)
                tanque_set_posicion(enemigo, old_ex, old_ey);

            float ep = tanque_phi(enemigo);
            float pdx = tanque_x(jugador) - ex;
            float pdy = tanque_y(jugador) - ey;
            float ang_jug = atan2f(pdy, pdx);
            float diff = ang_jug - ep;
            while (diff > M_PI) diff -= 2*M_PI;
            while (diff < -M_PI) diff += 2*M_PI;

            if (tanque_movimiento(enemigo) == MOV_NINGUNO) {
                if ((rand() % JUEGO_FPS) == 0) {
                    if (rand() % 2 == 0) {
                        float duracion = (rand() % 3001) / 1000.0f;
                        if (diff > 0)
                            tanque_iniciar_movimiento_tiempo(enemigo, MOV_GIRAR_DER, duracion);
                        else
                            tanque_iniciar_movimiento_tiempo(enemigo, MOV_GIRAR_IZQ, duracion);
                    } else {
                        float t = ((float)(rand() % 4001) - 1000.0f) / 1000.0f;
                        if (t < 0)
                            tanque_iniciar_movimiento_tiempo(enemigo, MOV_ATRAS, -t);
                        else
                            tanque_iniciar_movimiento_tiempo(enemigo, MOV_ADELANTE, t);
                    }
                }
            }

            if (fabs(diff) <= 1.0f) {
                float dtor = diff - tanque_torreta(enemigo);
                while (dtor > M_PI) dtor -= 2*M_PI;
                while (dtor < -M_PI) dtor += 2*M_PI;
                if (fabs(dtor) > 0.01f) {
                    if (dtor > 0)
                        tanque_girar_torreta(enemigo, 0.12f * dt);
                    else
                        tanque_girar_torreta(enemigo, -0.12f * dt);
                }
                if (fabs(dtor) < 0.1f && tanque_puede_disparar(enemigo))
                    tanque_disparar(enemigo);
            } else {
                float tr = tanque_torreta(enemigo);
                if (fabs(tr) > 0.01f) {
                    if (tr > 0)
                        tanque_girar_torreta(enemigo, -0.24f * dt);
                    else
                        tanque_girar_torreta(enemigo, 0.24f * dt);
                }
            }

            movimiento_e mov_act = tanque_movimiento(enemigo);
            if (mov_act != MOV_GIRAR_IZQ && mov_act != MOV_GIRAR_DER) {
                for (size_t i = 0; i < num_obs; i++) {
                    float dx = obstaculo_x(obstaculos[i]) - ex;
                    float dy = obstaculo_y(obstaculos[i]) - ey;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < 100.0f && d2 > 0.01f) {
                        float ang_obs = atan2f(dy, dx);
                        float diff_obs = ang_obs - ep;
                        while (diff_obs > M_PI) diff_obs -= 2*M_PI;
                        while (diff_obs < -M_PI) diff_obs += 2*M_PI;
                        if (fabs(diff_obs) < 1.5f) {
                            if (diff_obs > 0)
                                tanque_girar(enemigo, -0.36f * dt);
                            else
                                tanque_girar(enemigo, 0.36f * dt);
                            break;
                        }
                    }
                }
            }

            px = tanque_x(jugador);
            py = tanque_y(jugador);
            float pp = tanque_phi(jugador);

            matriz_t *cam = matriz_crear_identidad(4);

            float angx_rad = angx > 0.01f ? angx : 0;

            matriz_t *mper = matriz_crear_mper(4);
            matriz_t *tmp = matriz_multiplicar(cam, mper);
            matriz_destruir(cam);
            matriz_destruir(mper);
            cam = tmp;

            matriz_t *mz1 = matriz_crear_mz(M_PI / 2 + angz);
            tmp = matriz_multiplicar(cam, mz1);
            matriz_destruir(cam);
            matriz_destruir(mz1);
            cam = tmp;

            matriz_t *my1 = matriz_crear_my(M_PI / 2 - angx_rad);
            tmp = matriz_multiplicar(cam, my1);
            matriz_destruir(cam);
            matriz_destruir(my1);
            cam = tmp;

            matriz_t *mz2 = matriz_crear_mz(-pp);
            tmp = matriz_multiplicar(cam, mz2);
            matriz_destruir(cam);
            matriz_destruir(mz2);
            cam = tmp;

            float vec[3] = {-px, -py, -3.0f};
            matriz_t *mt = matriz_crear_mt(vec);
            tmp = matriz_multiplicar(cam, mt);
            matriz_destruir(cam);
            matriz_destruir(mt);
            cam = tmp;

            pila_apilar(stack, cam);

            if (modelo_horizonte) {
                SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0x00);
                renderizar_modelo(renderer, stack, modelo_horizonte, 0, 0, 0, 0, 0);
            }
            if (modelo_montanas) {
                SDL_SetRenderDrawColor(renderer, 0x40, 0x40, 0x40, 0x00);
                renderizar_modelo(renderer, stack, modelo_montanas, 0, 0, 0, 0, 0);
            }
            if (modelo_luna) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xCC, 0x00);
                renderizar_modelo(renderer, stack, modelo_luna, 0, 500, -50, 0, 0);
            }

            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
            for (size_t i = 0; i < num_obs; i++) {
                renderizar_modelo(renderer, stack, obstaculo_modelo(obstaculos[i]),
                                  obstaculo_x(obstaculos[i]),
                                  obstaculo_y(obstaculos[i]),
                                  0.0f,
                                  obstaculo_phi(obstaculos[i]),
                                  0);
            }

            if (modelo_tanque && modelo_torreta) {
                ex = tanque_x(enemigo);
                ey = tanque_y(enemigo);
                ep = tanque_phi(enemigo);
                float et = tanque_torreta(enemigo);

                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                renderizar_modelo(renderer, stack, modelo_tanque, ex, ey, 0.0f, ep, 0);

                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                renderizar_modelo(renderer, stack, modelo_torreta, ex, ey, 3.0f, ep + et, 0);

                if (modelo_radar) {
                    float radar_rot = SDL_GetTicks() / 500.0f;
                    float radar_x = ex + (-1.5f) * cosf(ep + et);
                    float radar_y = ey + (-1.5f) * sinf(ep + et);
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                    renderizar_modelo(renderer, stack, modelo_radar, radar_x, radar_y, 3.5f, ep + et + radar_rot, 0);
                }
            }

            if (modelo_misil && tanque_misil_activo(jugador)) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                renderizar_modelo(renderer, stack, modelo_misil,
                                  tanque_misil_x(jugador),
                                  tanque_misil_y(jugador),
                                  2.5f,
                                  tanque_misil_phi(jugador),
                                  0);
            }

            if (modelo_misil && tanque_misil_activo(enemigo)) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
                renderizar_modelo(renderer, stack, modelo_misil,
                                  tanque_misil_x(enemigo),
                                  tanque_misil_y(enemigo),
                                  2.5f,
                                  tanque_misil_phi(enemigo),
                                  0);
            }

            if (tanque_misil_activo(jugador)) {
                float mx = tanque_misil_x(jugador), my = tanque_misil_y(jugador);
                int choco = 0;
                for (size_t i = 0; i < num_obs && !choco; i++) {
                    float dx = mx - obstaculo_x(obstaculos[i]);
                    float dy = my - obstaculo_y(obstaculos[i]);
                    if (dx*dx + dy*dy < 9.0f) choco = 1;
                }
                float dx = mx - tanque_x(enemigo);
                float dy = my - tanque_y(enemigo);
                if (dx*dx + dy*dy < 9.0f) {
                    choco = 1;
                    tanque_recibir_impacto(enemigo);
                }
                if (choco) tanque_desactivar_misil(jugador);
            }

            if (tanque_misil_activo(enemigo)) {
                float mx = tanque_misil_x(enemigo), my = tanque_misil_y(enemigo);
                int choco = 0;
                for (size_t i = 0; i < num_obs && !choco; i++) {
                    float dx = mx - obstaculo_x(obstaculos[i]);
                    float dy = my - obstaculo_y(obstaculos[i]);
                    if (dx*dx + dy*dy < 9.0f) choco = 1;
                }
                float dx = mx - tanque_x(jugador);
                float dy = my - tanque_y(jugador);
                if (dx*dx + dy*dy < 9.0f) {
                    choco = 1;
                    tanque_recibir_impacto(jugador);
                    hit_timer = 1.0f;
                }
                if (choco) tanque_desactivar_misil(enemigo);
            }

            if (tanque_vidas(enemigo) <= 0) {
                puntaje += 1000;
                resto_x = tanque_x(enemigo);
                resto_y = tanque_y(enemigo);
                resto_timer = 1.5f;
                tanque_destruir(enemigo);
                enemigo = NULL;
                while (!enemigo) {
                    float ang = (rand() % 628) / 100.0f;
                    float dist = 50.0f;
                    float ex = tanque_x(jugador) + dist * cos(ang);
                    float ey = tanque_y(jugador) + dist * sin(ang);
                    enemigo = crear_tanque_enemigo(ex, ey, M_PI / 2, 1, obstaculos, num_obs);
                }
            }

            if (resto_timer > 0 && modelo_resto1) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
                float t = 1.5f - resto_timer;
                float rpx = 5.0f * t;
                float rpz = 10.0f * t - 0.5f * 9.81f * t * t;
                if (rpz < 0) rpz = 0;

                modelo_t *piezas[6] = {modelo_torreta, modelo_radar, modelo_resto1,
                                       modelo_resto2, modelo_resto1, modelo_resto2};
                for (int k = 0; k < 6; k++) {
                    if (!piezas[k]) continue;
                    float angulo = k * (2 * M_PI / 6);
                    float wx = resto_x + rpx * cosf(angulo);
                    float wy = resto_y + rpx * sinf(angulo);
                    float sr = (rand() % 628) / 1000.0f;
                    float sy = (rand() % 628) / 1000.0f;
                    float vec2[3] = {wx, wy, rpz};
                    matriz_t *mt2 = matriz_crear_mt(vec2);
                    matriz_t *mrz = matriz_crear_mz(angulo + sr);
                    matriz_t *mry = matriz_crear_my(sy);
                    matriz_t *obj1 = matriz_multiplicar(mt2, mrz);
                    matriz_t *obj = matriz_multiplicar(obj1, mry);
                    matriz_destruir(mt2);
                    matriz_destruir(mrz);
                    matriz_destruir(mry);
                    matriz_destruir(obj1);
                    matriz_t *cam2 = pila_ver_tope(stack);
                    matriz_t *final = matriz_multiplicar(cam2, obj);
                    pila_apilar(stack, final);
                    matriz_destruir(obj);
                    const float *coords = modelo_coords(piezas[k]);
                    const size_t *lineas = modelo_lineas(piezas[k]);
                    size_t ncoords = modelo_ncoords(piezas[k]);
                    size_t nlineas = modelo_nlineas(piezas[k]);
                    matriz_t *pts = _matriz_crear(ncoords, 3);
                    for (size_t i = 0; i < ncoords; i++)
                        matriz_establecer(pts, i, 0, coords[3*i]);
                    for (size_t i = 0; i < ncoords; i++)
                        matriz_establecer(pts, i, 1, coords[3*i+1]);
                    for (size_t i = 0; i < ncoords; i++)
                        matriz_establecer(pts, i, 2, coords[3*i+2]);
                    matriz_t *proj = matriz_aplicar(pila_ver_tope(stack), pts);
                    matriz_destruir(pts);
                    for (size_t i = 0; i < 2 * nlineas; i += 2) {
                        size_t i0 = lineas[i], i1 = lineas[i+1];
                        float d0 = matriz_obtener(proj, i0, 2);
                        float d1 = matriz_obtener(proj, i1, 2);
                        if (d0 >= 1.0f && d1 >= 1.0f) {
                            float x0 = matriz_obtener(proj, i0, 0);
                            float y0 = matriz_obtener(proj, i0, 1);
                            float x1 = matriz_obtener(proj, i1, 0);
                            float y1 = matriz_obtener(proj, i1, 1);
                            int sx0 = (int)(x0 * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
                            int sy0 = (int)(VENTANA_ALTO/2 - y0 * VENTANA_ALTO/2);
                            int sx1 = (int)(x1 * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
                            int sy1 = (int)(VENTANA_ALTO/2 - y1 * VENTANA_ALTO/2);
                            SDL_RenderDrawLine(renderer, sx0, sy0, sx1, sy1);
                        }
                    }
                    matriz_destruir(proj);
                    matriz_destruir(pila_desapilar(stack));
                }
            }

            matriz_destruir(pila_desapilar(stack));
        }

        float p_diff = 0;
        if (enemigo) {
            float p_ang = atan2f(tanque_y(enemigo) - tanque_y(jugador),
                                 tanque_x(enemigo) - tanque_x(jugador));
            p_diff = p_ang - tanque_phi(jugador);
            while (p_diff > M_PI) p_diff -= 2*M_PI;
            while (p_diff < -M_PI) p_diff += 2*M_PI;
        }

        if (!game_over) {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            int cx = VENTANA_ANCHO/2, cy = VENTANA_ALTO/2;
            if (fabs(p_diff) < 0.15f) {
                SDL_RenderDrawLine(renderer, cx-8, cy, cx+8, cy);
                SDL_RenderDrawLine(renderer, cx, cy-8, cx, cy+8);
            } else {
                SDL_RenderDrawLine(renderer, cx-8, cy, cx+8, cy);
            }
        }

        matriz_t *hud_cam = matriz_crear_identidad(4);

        matriz_t *mper = matriz_crear_mper(4);
        matriz_t *tmp = matriz_multiplicar(hud_cam, mper);
        matriz_destruir(hud_cam);
        matriz_destruir(mper);
        hud_cam = tmp;

        float zoff[3] = {0, 0, -20};
        matriz_t *mt = matriz_crear_mt(zoff);
        tmp = matriz_multiplicar(hud_cam, mt);
        matriz_destruir(hud_cam);
        matriz_destruir(mt);
        hud_cam = tmp;

        pila_apilar(stack, hud_cam);

        if (game_over) {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
            const char *msg = "GAME OVER";
            int len = strlen(msg);
            float start_x = -len * 0.7f / 2;
            for (int i = 0; i < len; i++) {
                if (msg[i] >= 'A' && msg[i] <= 'Z' && modelo_letras[msg[i] - 'A'])
                    renderizar_modelo(renderer, stack, modelo_letras[msg[i] - 'A'],
                                      start_x + i * 0.7f, 0.8f, 0, 0, 0);
            }
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            int temp = puntaje;
            char buf[10];
            int n = 0;
            if (temp == 0) {
                buf[n++] = '0';
            } else {
                while (temp > 0) {
                    buf[n++] = '0' + temp % 10;
                    temp /= 10;
                }
            }
            start_x = -n * 0.9f / 2;
            for (int i = n - 1; i >= 0; i--) {
                modelo_t *m = modelo_numeros[buf[i] - '0'];
                if (m) renderizar_modelo(renderer, stack, m, start_x + (n-1-i) * 0.9f, -0.3f, 0, 0, 0);
            }
        } else {
            if (hit_timer > 0 && modelo_hash) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
                size_t hash_lines = modelo_nlineas(modelo_hash);
                size_t show = (size_t)((1.0f - hit_timer) * hash_lines);
                if (show > hash_lines) show = hash_lines;
                renderizar_modelo(renderer, stack, modelo_hash, 0, 0, 0, 0, show);
            }

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            for (int i = 0; i < tanque_vidas(jugador) - 1 && modelo_star; i++)
                renderizar_modelo(renderer, stack, modelo_star, -6.0f + i * 1.5f, 3.8f, 0, 0, 0);

            int temp = puntaje;
            char buf[10];
            int n = 0;
            if (temp == 0) {
                buf[n++] = '0';
            } else {
                while (temp > 0) {
                    buf[n++] = '0' + temp % 10;
                    temp /= 10;
                }
            }
            float sx = 4.5f - (n - 1) * 0.9f;
            for (int i = n - 1; i >= 0; i--) {
                modelo_t *m = modelo_numeros[buf[i] - '0'];
                if (m) renderizar_modelo(renderer, stack, m, sx, 3.8f, 0, 0, 0);
                sx += 0.9f;
            }

            if (fabs(p_diff) > 1.0f && enemigo) {
                const char *msg = "ATRAS";
                if (p_diff > 1.0f && p_diff <= 2.44f)
                    msg = "IZQUIERDA";
                else if (p_diff < -1.0f && p_diff >= -2.44f)
                    msg = "DERECHA";
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                int len = strlen(msg);
                float start_x = -len * 0.7f / 2;
                for (int i = 0; i < len; i++) {
                    if (msg[i] >= 'A' && msg[i] <= 'Z' && modelo_letras[msg[i] - 'A'])
                        renderizar_modelo(renderer, stack, modelo_letras[msg[i] - 'A'],
                                          start_x + i * 0.7f, 0.5f, 0, 0, 0);
                }
            }

            float pp = tanque_phi(jugador);
            int compass_cx = VENTANA_ANCHO / 2;
            int compass_cy = VENTANA_ALTO - 50;
            int compass_r = 18;
            SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0x00);
            SDL_RenderDrawLine(renderer, compass_cx - compass_r, compass_cy, compass_cx + compass_r, compass_cy);
            SDL_RenderDrawLine(renderer, compass_cx, compass_cy - compass_r, compass_cx, compass_cy + compass_r);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            int tip_x = compass_cx + (int)(cosf(pp) * compass_r);
            int tip_y = compass_cy - (int)(sinf(pp) * compass_r);
            SDL_RenderDrawLine(renderer, compass_cx, compass_cy, tip_x, tip_y);
        }

        matriz_destruir(pila_desapilar(stack));

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

    lista_destruir(lista_modelos, (void (*)(void *))modelo_destruir);
    for (size_t i = 0; i < num_obs; i++)
        obstaculo_destruir(obstaculos[i]);
    tanque_destruir(jugador);
    tanque_destruir(enemigo);
    matriz_destruir(pila_desapilar(stack));
    pila_destruir(stack, NULL);

    // END código del alumno

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
