#include "juego.h"
#include "tanque.h"
#include "obstaculo.h"
#include "modelo.h"
#include "stl.h"
#include "matriz.h"
#include "pila.h"
#include "lista.h"
#include "cola.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14
#endif

#define MUNDO 150.0f
#define JUEGO_FPS 24

struct juego {
    tanque_t *jugador;
    tanque_t *enemigo;
    obstaculo_t **obstaculos;
    size_t num_obs;
    int puntaje;
    bool game_over;
    float hit_timer, resto_timer;
    float resto_x, resto_y;
    float angx, angz;
    unsigned int prev_ticks;

    modelo_t *modelo_tanque, *modelo_torreta, *modelo_radar;
    modelo_t *modelo_misil, *modelo_horizonte;
    modelo_t *modelo_montanas, *modelo_luna;
    modelo_t *obs_models[6];
    modelo_t *modelo_letras[26];
    modelo_t *modelo_numeros[10];
    modelo_t *modelo_star, *modelo_hash;
    modelo_t *modelo_resto1, *modelo_resto2;

    lista_t *lista_modelos;
    pila_t *stack;
};

static void clasificar_modelo(juego_t *j, modelo_t *m) {
    const char *name = modelo_nombre(m);
    if      (strcmp(name, "TANQUE") == 0)    j->modelo_tanque = m;
    else if (strcmp(name, "TORRETA") == 0)   j->modelo_torreta = m;
    else if (strcmp(name, "RADAR") == 0)     j->modelo_radar = m;
    else if (strcmp(name, "MISIL") == 0)     j->modelo_misil = m;
    else if (strcmp(name, "HORIZONTE") == 0) j->modelo_horizonte = m;
    else if (strcmp(name, "MONTANA") == 0)   j->modelo_montanas = m;
    else if (strcmp(name, "LUNA") == 0)      j->modelo_luna = m;
    else if (strcmp(name, "CUBO1") == 0)     j->obs_models[0] = m;
    else if (strcmp(name, "CUBO2") == 0)     j->obs_models[1] = m;
    else if (strcmp(name, "CUBO3") == 0)     j->obs_models[2] = m;
    else if (strcmp(name, "PIRAMIDE1") == 0) j->obs_models[3] = m;
    else if (strcmp(name, "PIRAMIDE2") == 0) j->obs_models[4] = m;
    else if (strcmp(name, "PIRAMIDE3") == 0) j->obs_models[5] = m;
    else if (strlen(name) == 1 && name[0] >= 'A' && name[0] <= 'Z')
        j->modelo_letras[name[0] - 'A'] = m;
    else if (strlen(name) == 1 && name[0] >= '0' && name[0] <= '9')
        j->modelo_numeros[name[0] - '0'] = m;
    else if (strcmp(name, "*") == 0)     j->modelo_star = m;
    else if (strcmp(name, "#") == 0)     j->modelo_hash = m;
    else if (strcmp(name, "RESTO1") == 0) j->modelo_resto1 = m;
    else if (strcmp(name, "RESTO2") == 0) j->modelo_resto2 = m;
}

juego_t *juego_crear(SDL_Renderer *renderer) {
    (void)renderer;
    juego_t *j = calloc(1, sizeof(juego_t));
    if (!j) return NULL;

    j->lista_modelos = lista_crear();
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

    while (!cola_esta_vacia(cola_modelos)) {
        modelo_t *m = cola_desencolar(cola_modelos);
        lista_insertar_ultimo(j->lista_modelos, m);
    }
    cola_destruir(cola_modelos, NULL);

    lista_iter_t *it = lista_iter_crear(j->lista_modelos);
    while (!lista_iter_al_final(it)) {
        clasificar_modelo(j, lista_iter_ver_actual(it));
        lista_iter_avanzar(it);
    }
    lista_iter_destruir(it);

    if (lista_largo(j->lista_modelos) == 0)
        fprintf(stderr, "ERROR: no se cargaron modelos desde modelos.stl\n");

    j->num_obs = 50;
    int obs_available = 0;
    for (int k = 0; k < 6; k++) {
        if (j->obs_models[k])
            obs_available = 1;
    }
    if (!obs_available) {
        j->num_obs = 0;
        fprintf(stderr, "ERROR: no hay modelos de obstaculos (CUBO1-3, PIRAMIDE1-3)\n");
    }

    j->obstaculos = malloc(sizeof(obstaculo_t *) * j->num_obs);
    for (size_t i = 0; i < j->num_obs; ) {
        int idx = rand() % 6;
        if (!j->obs_models[idx]) continue;
        float ox = (rand() % (int)(MUNDO * 200 + 1)) / 100.0f - MUNDO;
        float oy = (rand() % (int)(MUNDO * 200 + 1)) / 100.0f - MUNDO;
        float op = (rand() % 628) / 100.0f - M_PI;
        j->obstaculos[i] = obstaculo_crear(ox, oy, op, j->obs_models[idx]);
        i++;
    }

    j->jugador = tanque_crear(0, 0, M_PI / 2, 4);

    j->enemigo = NULL;
    while (!j->enemigo) {
        float ang = (rand() % 628) / 100.0f;
        float dist = 50.0f;
        float ex = tanque_x(j->jugador) + dist * cos(ang);
        float ey = tanque_y(j->jugador) + dist * sin(ang);
        j->enemigo = crear_tanque_enemigo(ex, ey, M_PI / 2, 1,
                                           j->obstaculos, j->num_obs);
    }

    j->prev_ticks = SDL_GetTicks();

    j->stack = pila_crear();
    matriz_t *ident = matriz_crear_identidad(4);
    pila_apilar(j->stack, ident);

    return j;
}

void juego_destruir(juego_t *j) {
    if (!j) return;
    lista_destruir(j->lista_modelos, (void (*)(void *))modelo_destruir);
    for (size_t i = 0; i < j->num_obs; i++)
        obstaculo_destruir(j->obstaculos[i]);
    free(j->obstaculos);
    tanque_destruir(j->jugador);
    tanque_destruir(j->enemigo);
    matriz_destruir(pila_desapilar(j->stack));
    pila_destruir(j->stack, NULL);
    free(j);
}

bool juego_tratar_evento(juego_t *j, const SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                tanque_iniciar_movimiento(j->jugador, MOV_ADELANTE);
                break;
            case SDLK_DOWN:
                tanque_iniciar_movimiento(j->jugador, MOV_ATRAS);
                break;
            case SDLK_RIGHT:
                tanque_iniciar_movimiento(j->jugador, MOV_GIRAR_IZQ);
                break;
            case SDLK_LEFT:
                tanque_iniciar_movimiento(j->jugador, MOV_GIRAR_DER);
                break;
            case ' ':
                if (tanque_puede_disparar(j->jugador))
                    tanque_disparar(j->jugador);
                break;
            default:
                break;
        }
    }
    return true;
}

static void respawn_enemigo(juego_t *j) {
    tanque_destruir(j->enemigo);
    j->enemigo = NULL;
    while (!j->enemigo) {
        float ang = (rand() % 628) / 100.0f;
        float dist = 50.0f;
        float ex = tanque_x(j->jugador) + dist * cos(ang);
        float ey = tanque_y(j->jugador) + dist * sin(ang);
        j->enemigo = crear_tanque_enemigo(ex, ey, M_PI / 2, 1,
                                           j->obstaculos, j->num_obs);
    }
}

void juego_actualizar(juego_t *j) {
    float dt = (SDL_GetTicks() - j->prev_ticks) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    j->prev_ticks = SDL_GetTicks();

    if (j->hit_timer > 0) j->hit_timer -= dt;
    if (j->resto_timer > 0) j->resto_timer -= dt;
    if (tanque_vidas(j->jugador) <= 0) j->game_over = true;

    if (!j->game_over) {

        movimiento_e mov = tanque_movimiento(j->jugador);
        if (mov != MOV_NINGUNO) {
            if (mov == MOV_GIRAR_IZQ || mov == MOV_GIRAR_DER)
                j->angz += ((rand() % 2001) - 1000) / 100000.0f;
            else
                j->angx += ((rand() % 2001) - 1000) / 100000.0f;
        }
        j->angx *= 0.92f;
        j->angz *= 0.92f;

        float old_px = tanque_x(j->jugador), old_py = tanque_y(j->jugador);
        float old_ex = tanque_x(j->enemigo), old_ey = tanque_y(j->enemigo);
        tanque_actualizar(j->jugador, dt);
        tanque_actualizar(j->enemigo, dt);

        float px = tanque_x(j->jugador), py = tanque_y(j->jugador);
        int choco_jugador = 0;
        for (size_t i = 0; i < j->num_obs && !choco_jugador; i++) {
            float dx = px - obstaculo_x(j->obstaculos[i]);
            float dy = py - obstaculo_y(j->obstaculos[i]);
            if (dx*dx + dy*dy < 36.0f) choco_jugador = 1;
        }
        if (!choco_jugador) {
            float dx = px - old_ex;
            float dy = py - old_ey;
            if (dx*dx + dy*dy < 36.0f) choco_jugador = 1;
        }
        if (choco_jugador)
            tanque_set_posicion(j->jugador, old_px, old_py);

        float ex = tanque_x(j->enemigo), ey = tanque_y(j->enemigo);
        int choco_enemigo = 0;
        for (size_t i = 0; i < j->num_obs && !choco_enemigo; i++) {
            float dx = ex - obstaculo_x(j->obstaculos[i]);
            float dy = ey - obstaculo_y(j->obstaculos[i]);
            if (dx*dx + dy*dy < 36.0f) choco_enemigo = 1;
        }
        if (!choco_enemigo) {
            float dx = ex - tanque_x(j->jugador);
            float dy = ey - tanque_y(j->jugador);
            if (dx*dx + dy*dy < 36.0f) choco_enemigo = 1;
        }
        if (choco_enemigo)
            tanque_set_posicion(j->enemigo, old_ex, old_ey);

        float ep = tanque_phi(j->enemigo);
        float pdx = tanque_x(j->jugador) - ex;
        float pdy = tanque_y(j->jugador) - ey;
        float ang_jug = atan2f(pdy, pdx);
        float diff = ang_jug - ep;
        while (diff > M_PI) diff -= 2*M_PI;
        while (diff < -M_PI) diff += 2*M_PI;

        if (tanque_movimiento(j->enemigo) == MOV_NINGUNO) {
            if ((rand() % JUEGO_FPS) == 0) {
                if (rand() % 2 == 0) {
                    float duracion = (rand() % 3001) / 1000.0f;
                    if (diff > 0)
                        tanque_iniciar_movimiento_tiempo(j->enemigo, MOV_GIRAR_DER, duracion);
                    else
                        tanque_iniciar_movimiento_tiempo(j->enemigo, MOV_GIRAR_IZQ, duracion);
                } else {
                    float t = ((float)(rand() % 4001) - 1000.0f) / 1000.0f;
                    if (t < 0)
                        tanque_iniciar_movimiento_tiempo(j->enemigo, MOV_ATRAS, -t);
                    else
                        tanque_iniciar_movimiento_tiempo(j->enemigo, MOV_ADELANTE, t);
                }
            }
        }

        if (fabs(diff) <= 1.0f) {
            float dtor = diff - tanque_torreta(j->enemigo);
            while (dtor > M_PI) dtor -= 2*M_PI;
            while (dtor < -M_PI) dtor += 2*M_PI;
            if (fabs(dtor) > 0.01f) {
                if (dtor > 0)
                    tanque_girar_torreta(j->enemigo, 0.12f * dt);
                else
                    tanque_girar_torreta(j->enemigo, -0.12f * dt);
            }
            if (fabs(dtor) < 0.1f && tanque_puede_disparar(j->enemigo))
                tanque_disparar(j->enemigo);
        } else {
            float tr = tanque_torreta(j->enemigo);
            if (fabs(tr) > 0.01f) {
                if (tr > 0)
                    tanque_girar_torreta(j->enemigo, -0.24f * dt);
                else
                    tanque_girar_torreta(j->enemigo, 0.24f * dt);
            }
        }

        if (tanque_misil_activo(j->jugador)) {
            float mx = tanque_misil_x(j->jugador), my = tanque_misil_y(j->jugador);
            int choco = 0;
            for (size_t i = 0; i < j->num_obs && !choco; i++) {
                float dx = mx - obstaculo_x(j->obstaculos[i]);
                float dy = my - obstaculo_y(j->obstaculos[i]);
                if (dx*dx + dy*dy < 9.0f) choco = 1;
            }
            float dx = mx - tanque_x(j->enemigo);
            float dy = my - tanque_y(j->enemigo);
            if (dx*dx + dy*dy < 9.0f) {
                choco = 1;
                tanque_recibir_impacto(j->enemigo);
            }
            if (choco) tanque_desactivar_misil(j->jugador);
        }

        if (tanque_misil_activo(j->enemigo)) {
            float mx = tanque_misil_x(j->enemigo), my = tanque_misil_y(j->enemigo);
            int choco = 0;
            for (size_t i = 0; i < j->num_obs && !choco; i++) {
                float dx = mx - obstaculo_x(j->obstaculos[i]);
                float dy = my - obstaculo_y(j->obstaculos[i]);
                if (dx*dx + dy*dy < 9.0f) choco = 1;
            }
            float dx = mx - tanque_x(j->jugador);
            float dy = my - tanque_y(j->jugador);
            if (dx*dx + dy*dy < 9.0f) {
                choco = 1;
                tanque_recibir_impacto(j->jugador);
                j->hit_timer = 1.0f;
            }
            if (choco) tanque_desactivar_misil(j->enemigo);
        }

        if (tanque_vidas(j->enemigo) <= 0) {
            j->puntaje += 1000;
            j->resto_x = tanque_x(j->enemigo);
            j->resto_y = tanque_y(j->enemigo);
            j->resto_timer = 1.5f;
            respawn_enemigo(j);
        }
    }
}

bool   juego_terminado(const juego_t *j)       { return j->game_over; }
int    juego_puntaje(const juego_t *j)          { return j->puntaje; }
float  juego_hit_timer(const juego_t *j)        { return j->hit_timer; }
float  juego_resto_timer(const juego_t *j)      { return j->resto_timer; }
float  juego_resto_x(const juego_t *j)          { return j->resto_x; }
float  juego_resto_y(const juego_t *j)          { return j->resto_y; }
float  juego_angx(const juego_t *j)             { return j->angx; }
float  juego_angz(const juego_t *j)             { return j->angz; }

float  juego_jugador_x(const juego_t *j)        { return tanque_x(j->jugador); }
float  juego_jugador_y(const juego_t *j)        { return tanque_y(j->jugador); }
float  juego_jugador_phi(const juego_t *j)      { return tanque_phi(j->jugador); }
int    juego_jugador_vidas(const juego_t *j)    { return tanque_vidas(j->jugador); }

bool   juego_enemigo_existe(const juego_t *j)   { return j->enemigo != NULL; }
float  juego_enemigo_x(const juego_t *j)        { return tanque_x(j->enemigo); }
float  juego_enemigo_y(const juego_t *j)        { return tanque_y(j->enemigo); }
float  juego_enemigo_phi(const juego_t *j)      { return tanque_phi(j->enemigo); }
float  juego_enemigo_torreta(const juego_t *j)  { return tanque_torreta(j->enemigo); }

bool   juego_misil_jugador_activo(const juego_t *j)  { return tanque_misil_activo(j->jugador); }
float  juego_misil_jugador_x(const juego_t *j)       { return tanque_misil_x(j->jugador); }
float  juego_misil_jugador_y(const juego_t *j)       { return tanque_misil_y(j->jugador); }
float  juego_misil_jugador_phi(const juego_t *j)     { return tanque_misil_phi(j->jugador); }

bool   juego_misil_enemigo_activo(const juego_t *j)  { return j->enemigo && tanque_misil_activo(j->enemigo); }
float  juego_misil_enemigo_x(const juego_t *j)       { return tanque_misil_x(j->enemigo); }
float  juego_misil_enemigo_y(const juego_t *j)       { return tanque_misil_y(j->enemigo); }
float  juego_misil_enemigo_phi(const juego_t *j)     { return tanque_misil_phi(j->enemigo); }

size_t juego_num_obstaculos(const juego_t *j)              { return j->num_obs; }
const obstaculo_t *juego_obstaculo(const juego_t *j, size_t i) { return j->obstaculos[i]; }

pila_t *juego_stack(const juego_t *j) { return j->stack; }

const modelo_t *juego_modelo(const juego_t *j, const char *nombre) {
    if (strcmp(nombre, "TANQUE") == 0)    return j->modelo_tanque;
    if (strcmp(nombre, "TORRETA") == 0)   return j->modelo_torreta;
    if (strcmp(nombre, "RADAR") == 0)     return j->modelo_radar;
    if (strcmp(nombre, "MISIL") == 0)     return j->modelo_misil;
    if (strcmp(nombre, "HORIZONTE") == 0) return j->modelo_horizonte;
    if (strcmp(nombre, "MONTANA") == 0)   return j->modelo_montanas;
    if (strcmp(nombre, "LUNA") == 0)      return j->modelo_luna;
    if (strcmp(nombre, "RESTO1") == 0)    return j->modelo_resto1;
    if (strcmp(nombre, "RESTO2") == 0)    return j->modelo_resto2;
    if (strcmp(nombre, "*") == 0)         return j->modelo_star;
    if (strcmp(nombre, "#") == 0)         return j->modelo_hash;
    if (strlen(nombre) == 1 && nombre[0] >= 'A' && nombre[0] <= 'Z')
        return j->modelo_letras[nombre[0] - 'A'];
    if (strlen(nombre) == 1 && nombre[0] >= '0' && nombre[0] <= '9')
        return j->modelo_numeros[nombre[0] - '0'];
    if (strlen(nombre) >= 4 && strncmp(nombre, "CUBO", 4) == 0) {
        int idx = nombre[4] - '1';
        if (idx >= 0 && idx < 6) return j->obs_models[idx];
    }
    if (strlen(nombre) >= 8 && strncmp(nombre, "PIRAMIDE", 8) == 0) {
        int idx = nombre[8] - '1';
        if (idx >= 0 && idx < 6) return j->obs_models[3 + idx];
    }
    return NULL;
}
