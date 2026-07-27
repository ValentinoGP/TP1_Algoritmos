#include <SDL2/SDL.h>
#include "juego.h"
#include "modelo.h"
#include "obstaculo.h"
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
#ifndef M_PI
#define M_PI 3.14
#endif

typedef struct {
    const modelo_t *horizonte;
    const modelo_t *montana;
    const modelo_t *luna;
    const modelo_t *tanque;
    const modelo_t *torreta;
    const modelo_t *radar;
    const modelo_t *misil;
    const modelo_t *star;
    const modelo_t *hash;
    const modelo_t *resto1;
    const modelo_t *resto2;
    const modelo_t *letras[26];
    const modelo_t *digitos[10];
} model_cache_t;

static void cache_modelos(model_cache_t *c, juego_t *j) {
    c->horizonte = juego_modelo(j, "HORIZONTE");
    c->montana   = juego_modelo(j, "MONTANA");
    c->luna      = juego_modelo(j, "LUNA");
    c->tanque    = juego_modelo(j, "TANQUE");
    c->torreta   = juego_modelo(j, "TORRETA");
    c->radar     = juego_modelo(j, "RADAR");
    c->misil     = juego_modelo(j, "MISIL");
    c->star      = juego_modelo(j, "*");
    c->hash      = juego_modelo(j, "#");
    c->resto1    = juego_modelo(j, "RESTO1");
    c->resto2    = juego_modelo(j, "RESTO2");
    for (int i = 0; i < 26; i++) {
        char name[2] = {'A' + i, '\0'};
        c->letras[i] = juego_modelo(j, name);
    }
    for (int i = 0; i < 10; i++) {
        char name[2] = {'0' + i, '\0'};
        c->digitos[i] = juego_modelo(j, name);
    }
}

static void renderizar_modelo(SDL_Renderer *r, pila_t *p, const modelo_t *modelo,
                               float x, float y, float z, float rot,
                               size_t max_lineas) {
    if (!modelo) return;

    const float *cam = matriz_datos(pila_ver_tope(p));

    float c = cosf(rot), s = sinf(rot);
    float obj[16] = {
        c, -s, 0, x,
        s,  c, 0, y,
        0,  0, 1, z,
        0,  0, 0, 1
    };

    float t[16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            float v = 0;
            for (int k = 0; k < 4; k++)
                v += cam[i*4+k] * obj[k*4+j];
            t[i*4+j] = v;
        }

    const float *coords = modelo_coords(modelo);
    const size_t *lineas = modelo_lineas(modelo);
    size_t nlineas = modelo_nlineas(modelo);
    if (max_lineas == 0 || max_lineas > nlineas) max_lineas = nlineas;

    for (size_t i = 0; i < 2 * max_lineas; i += 2) {
        size_t i0 = lineas[i], i1 = lineas[i+1];

        float px0[4] = {coords[3*i0], coords[3*i0+1], coords[3*i0+2], 1};
        float py0[4];
        for (int row = 0; row < 4; row++) {
            float v = 0;
            for (int k = 0; k < 4; k++) v += t[row*4+k] * px0[k];
            py0[row] = v;
        }
        if (py0[3] == 0) continue;
        float d0z = py0[3];
        float x0 = py0[0] / py0[3];
        float y0 = py0[1] / py0[3];

        float px1[4] = {coords[3*i1], coords[3*i1+1], coords[3*i1+2], 1};
        float py1[4];
        for (int row = 0; row < 4; row++) {
            float v = 0;
            for (int k = 0; k < 4; k++) v += t[row*4+k] * px1[k];
            py1[row] = v;
        }
        if (py1[3] == 0) continue;
        float d1z = py1[3];
        float x1 = py1[0] / py1[3];
        float y1 = py1[1] / py1[3];

        if (d0z >= 1.0f && d1z >= 1.0f) {
            int sx0 = (int)(x0 * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
            int sy0 = (int)(VENTANA_ALTO/2 - y0 * VENTANA_ALTO/2);
            int sx1 = (int)(x1 * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
            int sy1 = (int)(VENTANA_ALTO/2 - y1 * VENTANA_ALTO/2);
            SDL_RenderDrawLine(r, sx0, sy0, sx1, sy1);
        }
    }
}

static void construir_camara(juego_t *j, pila_t *stack) {
    float px = juego_jugador_x(j);
    float py = juego_jugador_y(j);
    float pp = juego_jugador_phi(j);
    float angx_rad = juego_angx(j);
    float angz_val = juego_angz(j);

    if (angx_rad <= 0.01f) angx_rad = 0;

    matriz_t *cam = matriz_crear_identidad(4);
    matriz_t *tmp;

    matriz_t *mper = matriz_crear_mper(1.85f);
    tmp = matriz_multiplicar(cam, mper);
    matriz_destruir(cam);
    matriz_destruir(mper);
    cam = tmp;

    matriz_t *mz1 = matriz_crear_mz(M_PI / 2 + angz_val);
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
}

static void construir_hud_cam(pila_t *stack) {
    matriz_t *hud_cam = matriz_crear_identidad(4);
    matriz_t *tmp;

    matriz_t *mper = matriz_crear_mper(1.85f);
    tmp = matriz_multiplicar(hud_cam, mper);
    matriz_destruir(hud_cam);
    matriz_destruir(mper);
    hud_cam = tmp;

    float zoff[3] = {0, 0, -12};
    matriz_t *mt = matriz_crear_mt(zoff);
    tmp = matriz_multiplicar(hud_cam, mt);
    matriz_destruir(hud_cam);
    matriz_destruir(mt);
    hud_cam = tmp;

    pila_apilar(stack, hud_cam);
}

static void renderizar_explosion(SDL_Renderer *renderer, pila_t *stack,
                                   juego_t *j, const model_cache_t *cache) {
    if (juego_resto_timer(j) <= 0) return;

    const modelo_t *m_resto1 = cache->resto1;
    if (!m_resto1) return;

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
    float t = 1.5f - juego_resto_timer(j);
    float rpx = 5.0f * t;
    float rpz = 10.0f * t - 0.5f * 9.81f * t * t;
    if (rpz < 0) rpz = 0;

    const modelo_t *piezas[6];
    piezas[0] = cache->torreta;
    piezas[1] = cache->radar;
    piezas[2] = m_resto1;
    piezas[3] = cache->resto2;
    piezas[4] = m_resto1;
    piezas[5] = cache->resto2;

    float rx = juego_resto_x(j);
    float ry = juego_resto_y(j);

    for (int k = 0; k < 6; k++) {
        if (!piezas[k]) continue;
        float angulo = k * (2 * M_PI / 6);
        float wx = rx + rpx * cosf(angulo);
        float wy = ry + rpx * sinf(angulo);
        renderizar_modelo(renderer, stack, piezas[k], wx, wy, rpz, angulo, 0);
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Battle Zone");

    juego_t *juego = juego_crear(renderer);
    pila_t *stack = juego_stack(juego);

    model_cache_t cache;
    cache_modelos(&cache, juego);

    unsigned int ticks = SDL_GetTicks();
    int done = 0;

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
                break;
            }
            juego_tratar_evento(juego, &event);
        }
        if (done) break;

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        juego_actualizar(juego);

        if (!juego_terminado(juego)) {

            construir_camara(juego, stack);

            if (cache.horizonte) {
                SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0x00);
                renderizar_modelo(renderer, stack, cache.horizonte, 0, 0, 0, 0, 0);
            }
            if (cache.montana) {
                SDL_SetRenderDrawColor(renderer, 0x40, 0x40, 0x40, 0x00);
                renderizar_modelo(renderer, stack, cache.montana, 0, 0, 0, 0, 0);
            }
            if (cache.luna) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xCC, 0x00);
                renderizar_modelo(renderer, stack, cache.luna, 0, 500, -50, 0, 0);
            }

            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
            for (size_t i = 0; i < juego_num_obstaculos(juego); i++) {
                const obstaculo_t *obs = juego_obstaculo(juego, i);
                renderizar_modelo(renderer, stack, obstaculo_modelo(obs),
                                  obstaculo_x(obs), obstaculo_y(obs),
                                  0.0f, obstaculo_phi(obs), 0);
            }

            if (juego_enemigo_existe(juego)) {
                float ex = juego_enemigo_x(juego);
                float ey = juego_enemigo_y(juego);
                float ep = juego_enemigo_phi(juego);
                float et = juego_enemigo_torreta(juego);

                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                renderizar_modelo(renderer, stack, cache.tanque,
                                  ex, ey, 0.0f, ep, 0);
                renderizar_modelo(renderer, stack, cache.torreta,
                                  ex, ey, 3.0f, ep + et, 0);

                if (cache.radar) {
                    float radar_rot = SDL_GetTicks() / 500.0f;
                    float radar_x = ex + (-1.5f) * cosf(ep + et);
                    float radar_y = ey + (-1.5f) * sinf(ep + et);
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                    renderizar_modelo(renderer, stack, cache.radar,
                                      radar_x, radar_y, 3.5f,
                                      ep + et + radar_rot, 0);
                }
            }

            if (cache.misil && juego_misil_jugador_activo(juego)) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                renderizar_modelo(renderer, stack, cache.misil,
                                  juego_misil_jugador_x(juego),
                                  juego_misil_jugador_y(juego),
                                  2.5f,
                                  juego_misil_jugador_phi(juego), 0);
            }
            if (cache.misil && juego_misil_enemigo_activo(juego)) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
                renderizar_modelo(renderer, stack, cache.misil,
                                  juego_misil_enemigo_x(juego),
                                  juego_misil_enemigo_y(juego),
                                  2.5f,
                                  juego_misil_enemigo_phi(juego), 0);
            }

            renderizar_explosion(renderer, stack, juego, &cache);

            matriz_destruir(pila_desapilar(stack));
        }

        float p_diff = 0;
        if (juego_enemigo_existe(juego)) {
            float p_ang = atan2f(juego_enemigo_y(juego) - juego_jugador_y(juego),
                                 juego_enemigo_x(juego) - juego_jugador_x(juego));
            p_diff = p_ang - juego_jugador_phi(juego);
            while (p_diff > M_PI) p_diff -= 2*M_PI;
            while (p_diff < -M_PI) p_diff += 2*M_PI;
        }

        if (!juego_terminado(juego)) {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            int cx = VENTANA_ANCHO/2, cy = VENTANA_ALTO/2;
            if (fabs(p_diff) < 0.15f) {
                SDL_RenderDrawLine(renderer, cx-8, cy, cx+8, cy);
                SDL_RenderDrawLine(renderer, cx, cy-8, cx, cy+8);
            } else {
                SDL_RenderDrawLine(renderer, cx-8, cy, cx+8, cy);
            }
        }

        construir_hud_cam(stack);

        if (juego_terminado(juego)) {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
            const char *msg = "GAME OVER";
            int len = strlen(msg);
            float start_x = -len * 0.9f / 2;
            for (int i = 0; i < len; i++) {
                int idx = msg[i] - 'A';
                const modelo_t *lt = (idx >= 0 && idx < 26) ? cache.letras[idx] : NULL;
                if (lt)
                    renderizar_modelo(renderer, stack, lt,
                                      start_x + i * 0.9f, 1.04f, 0, 0, 0);
            }
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            int temp = juego_puntaje(juego);
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
            start_x = -n * 1.17f / 2;
            for (int i = n - 1; i >= 0; i--) {
                int idx = buf[i] - '0';
                const modelo_t *dg = (idx >= 0 && idx < 10) ? cache.digitos[idx] : NULL;
                if (dg) renderizar_modelo(renderer, stack, dg,
                                          start_x + (n-1-i) * 1.17f, -0.39f, 0, 0, 0);
            }
        } else {
            if (juego_hit_timer(juego) > 0 && cache.hash) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
                size_t hash_lines = modelo_nlineas(cache.hash);
                size_t show = (size_t)((1.0f - juego_hit_timer(juego)) * hash_lines);
                if (show > hash_lines) show = hash_lines;
                renderizar_modelo(renderer, stack, cache.hash, 0, 0, 0, 0, show);
            }

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            for (int i = 0; i < juego_jugador_vidas(juego) - 1 && cache.star; i++)
                renderizar_modelo(renderer, stack, cache.star, -7.8f + i * 1.95f, 4.92f, 0, 0, 0);

            int temp = juego_puntaje(juego);
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
            float sx = 5.82f - (n - 1) * 1.17f;
            for (int i = n - 1; i >= 0; i--) {
                int idx = buf[i] - '0';
                const modelo_t *dg = (idx >= 0 && idx < 10) ? cache.digitos[idx] : NULL;
                if (dg) renderizar_modelo(renderer, stack, dg, sx, 4.92f, 0, 0, 0);
                sx += 1.17f;
            }

            if (fabs(p_diff) > 1.0f && juego_enemigo_existe(juego)) {
                const char *dir = "ATRAS";
                if (p_diff > 1.0f && p_diff <= 2.44f)
                    dir = "IZQUIERDA";
                else if (p_diff < -1.0f && p_diff >= -2.44f)
                    dir = "DERECHA";
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
                int len = strlen(dir);
                float start_x = -len * 0.9f / 2;
                for (int i = 0; i < len; i++) {
                    int idx = dir[i] - 'A';
                    const modelo_t *lt = (idx >= 0 && idx < 26) ? cache.letras[idx] : NULL;
                    if (lt)
                        renderizar_modelo(renderer, stack, lt,
                                          start_x + i * 0.9f, 0.65f, 0, 0, 0);
                }
            }

            float pp = juego_jugador_phi(juego);
            int compass_cx = VENTANA_ANCHO / 2;
            int compass_cy = VENTANA_ALTO - 50;
            int compass_r = 18;
            SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0x00);
            SDL_RenderDrawLine(renderer, compass_cx - compass_r, compass_cy,
                               compass_cx + compass_r, compass_cy);
            SDL_RenderDrawLine(renderer, compass_cx, compass_cy - compass_r,
                               compass_cx, compass_cy + compass_r);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
            int tip_x = compass_cx + (int)(cosf(pp) * compass_r);
            int tip_y = compass_cy - (int)(sinf(pp) * compass_r);
            SDL_RenderDrawLine(renderer, compass_cx, compass_cy, tip_x, tip_y);
        }

        matriz_destruir(pila_desapilar(stack));

        SDL_RenderPresent(renderer);
        ticks = SDL_GetTicks() - ticks;
        if (ticks < 1000 / JUEGO_FPS)
            SDL_Delay(1000 / JUEGO_FPS - ticks);
        else
            printf("Perdiendo cuadros\n");
        ticks = SDL_GetTicks();
    }

    juego_destruir(juego);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
