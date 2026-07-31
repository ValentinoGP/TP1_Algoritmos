#include "juego.h"
#include "tanque.h"
#include "obstaculo.h"
#include "modelo.h"
#include "stl.h"
#include "lista.h"
#include "misil.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14
#endif

#define MUNDO 150.0f
#define FPS_JUEGO 24

static const char *NOMBRES_OBSTACULOS[] = {
    "CUBO1", "CUBO2", "CUBO3", "PIRAMIDE1", "PIRAMIDE2", "PIRAMIDE3"
};

struct juego {
    tanque_t *jugador;
    tanque_t *enemigo;
    obstaculo_t **obstaculos;
    size_t cantidad_obstaculos;
    int puntaje;
    bool terminado;
    float tiempo_impacto, tiempo_resto;
    float resto_x, resto_y;
    float angx, angz;
    unsigned int tick_anterior;

    lista_t *lista_modelos;
    misil_t *misil_jugador;
    misil_t *misil_enemigo;
};

static const modelo_t *buscar_modelo(lista_t *lista, const char *nombre) {
    lista_iter_t *iter = lista_iter_crear(lista);
    if (!iter) return NULL;
    const modelo_t *encontrado = NULL;
    while (!lista_iter_al_final(iter)) {
        modelo_t *modelo = lista_iter_ver_actual(iter);
        if (strcmp(modelo_nombre(modelo), nombre) == 0) {
            encontrado = modelo;
            break;
        }
        lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
    return encontrado;
}

static void crear_enemigo_aleatorio(juego_t *juego) {
    juego->enemigo = NULL;
    while (!juego->enemigo) {
        float angulo = (rand() % 628) / 100.0f;
        float distancia = 50.0f;
        float enemigo_x = tanque_x(juego->jugador) + distancia * cos(angulo);
        float enemigo_y = tanque_y(juego->jugador) + distancia * sin(angulo);
        juego->enemigo = crear_tanque_enemigo(enemigo_x, enemigo_y, M_PI / 2, 1,
                                              juego->obstaculos, juego->cantidad_obstaculos);
    }
}

static bool hay_colision(const juego_t *juego, float x, float y,
                         float otro_x, float otro_y) {
    for (size_t i = 0; i < juego->cantidad_obstaculos; i++) {
        float distancia_x = x - obstaculo_x(juego->obstaculos[i]);
        float distancia_y = y - obstaculo_y(juego->obstaculos[i]);
        if (distancia_x*distancia_x + distancia_y*distancia_y < 36.0f) return true;
    }
    float distancia_x = x - otro_x;
    float distancia_y = y - otro_y;
    return distancia_x*distancia_x + distancia_y*distancia_y < 36.0f;
}

static bool misil_impacta(juego_t *juego, misil_t *misil, tanque_t *objetivo) {
    if (!misil_activo(misil)) return false;
    float pos_x = misil_x(misil), pos_y = misil_y(misil);
    for (size_t i = 0; i < juego->cantidad_obstaculos; i++) {
        float distancia_x = pos_x - obstaculo_x(juego->obstaculos[i]);
        float distancia_y = pos_y - obstaculo_y(juego->obstaculos[i]);
        if (distancia_x*distancia_x + distancia_y*distancia_y < 9.0f) {
            misil_desactivar(misil);
            return false;
        }
    }
    float distancia_x = pos_x - tanque_x(objetivo);
    float distancia_y = pos_y - tanque_y(objetivo);
    if (distancia_x*distancia_x + distancia_y*distancia_y < 9.0f) {
        misil_desactivar(misil);
        tanque_recibir_impacto(objetivo);
        return true;
    }
    return false;
}

juego_t *juego_crear(void) {
    juego_t *juego = calloc(1, sizeof(juego_t));
    if (!juego) return NULL;

    juego->lista_modelos = lista_crear();

    FILE *archivo = fopen("modelos.stl", "rb");
    if (archivo) {
        unidades_t unidades;
        size_t maxlong;
        leer_encabezado_stl(archivo);
        leer_formato_STL(archivo, &unidades, &maxlong);

        while (1) {
            float *coordenadas = NULL;
            size_t cantidad_coordenadas = 0;
            size_t *lineas = NULL;
            size_t cantidad_lineas = 0;
            char etiqueta[maxlong + 1];

            if (!leer_modelo_3d(archivo, maxlong, etiqueta, &cantidad_coordenadas,
                                &coordenadas, &cantidad_lineas, &lineas))
                break;

            etiqueta[maxlong] = '\0';
            modelo_t *modelo = modelo_crear(etiqueta, coordenadas, cantidad_coordenadas,
                                            lineas, cantidad_lineas);
            free(coordenadas);
            free(lineas);

            if (modelo)
                lista_insertar_ultimo(juego->lista_modelos, modelo);
        }
        fclose(archivo);
    }

    srand(SDL_GetTicks());

    if (lista_largo(juego->lista_modelos) == 0)
        fprintf(stderr, "ERROR: no se cargaron modelos desde modelos.stl\n");

    juego->cantidad_obstaculos = 50;
    const modelo_t *modelos_obstaculos[6] = {NULL};
    int hay_obstaculos = 0;
    for (int i = 0; i < 6; i++) {
        modelos_obstaculos[i] = buscar_modelo(juego->lista_modelos, NOMBRES_OBSTACULOS[i]);
        if (modelos_obstaculos[i])
            hay_obstaculos = 1;
    }
    if (!hay_obstaculos) {
        juego->cantidad_obstaculos = 0;
        fprintf(stderr, "ERROR: no hay modelos de obstaculos (CUBO1-3, PIRAMIDE1-3)\n");
    }

    juego->obstaculos = malloc(sizeof(obstaculo_t *) * juego->cantidad_obstaculos);
    for (size_t i = 0; i < juego->cantidad_obstaculos; ) {
        int indice = rand() % 6;
        if (!modelos_obstaculos[indice]) continue;
        float pos_x = (rand() % (int)(MUNDO * 200 + 1)) / 100.0f - MUNDO;
        float pos_y = (rand() % (int)(MUNDO * 200 + 1)) / 100.0f - MUNDO;
        float rotacion = (rand() % 628) / 100.0f - M_PI;
        juego->obstaculos[i] = obstaculo_crear(pos_x, pos_y, rotacion,
                                               modelos_obstaculos[indice]);
        i++;
    }

    juego->jugador = tanque_crear(0, 0, M_PI / 2, 4);

    crear_enemigo_aleatorio(juego);

    juego->tick_anterior = SDL_GetTicks();

    juego->misil_jugador = misil_crear();
    juego->misil_enemigo = misil_crear();
    if (!juego->misil_jugador || !juego->misil_enemigo) {
        juego_destruir(juego);
        return NULL;
    }

    return juego;
}

void juego_destruir(juego_t *juego) {
    if (!juego) return;
    lista_destruir(juego->lista_modelos, (void (*)(void *))modelo_destruir);
    for (size_t i = 0; i < juego->cantidad_obstaculos; i++)
        obstaculo_destruir(juego->obstaculos[i]);
    free(juego->obstaculos);
    tanque_destruir(juego->jugador);
    tanque_destruir(juego->enemigo);
    misil_destruir(juego->misil_jugador);
    misil_destruir(juego->misil_enemigo);
    free(juego);
}

bool juego_tratar_evento(juego_t *juego, const SDL_Event *evento) {
    if (evento->type == SDL_KEYDOWN) {
        switch (evento->key.keysym.sym) {
            case SDLK_UP:
                tanque_iniciar_movimiento(juego->jugador, MOV_ADELANTE);
                break;
            case SDLK_DOWN:
                tanque_iniciar_movimiento(juego->jugador, MOV_ATRAS);
                break;
            case SDLK_RIGHT:
                tanque_iniciar_movimiento(juego->jugador, MOV_GIRAR_IZQ);
                break;
            case SDLK_LEFT:
                tanque_iniciar_movimiento(juego->jugador, MOV_GIRAR_DER);
                break;
            case ' ':
                if (!misil_activo(juego->misil_jugador) && tanque_disparar(juego->jugador))
                    misil_lanzar(juego->misil_jugador, tanque_x(juego->jugador),
                                 tanque_y(juego->jugador),
                                 tanque_phi(juego->jugador) + tanque_torreta(juego->jugador));
                break;
            default:
                break;
        }
    }
    return true;
}

static void reaparecer_enemigo(juego_t *juego) {
    tanque_destruir(juego->enemigo);
    crear_enemigo_aleatorio(juego);
}

void juego_actualizar(juego_t *juego) {
    float delta_tiempo = (SDL_GetTicks() - juego->tick_anterior) / 1000.0f;
    if (delta_tiempo > 0.1f) delta_tiempo = 0.1f;
    juego->tick_anterior = SDL_GetTicks();

    if (juego->tiempo_impacto > 0) juego->tiempo_impacto -= delta_tiempo;
    if (juego->tiempo_resto > 0) juego->tiempo_resto -= delta_tiempo;
    if (tanque_vidas(juego->jugador) <= 0) juego->terminado = true;

    if (!juego->terminado) {

        movimiento_e movimiento = tanque_movimiento(juego->jugador);
        if (movimiento != MOV_NINGUNO) {
            if (movimiento == MOV_GIRAR_IZQ || movimiento == MOV_GIRAR_DER)
                juego->angz += ((rand() % 2001) - 1000) / 100000.0f;
            else
                juego->angx += ((rand() % 2001) - 1000) / 100000.0f;
        }
        juego->angx *= 0.92f;
        juego->angz *= 0.92f;

        float jugador_x_anterior = tanque_x(juego->jugador), jugador_y_anterior = tanque_y(juego->jugador);
        float enemigo_x_anterior = tanque_x(juego->enemigo), enemigo_y_anterior = tanque_y(juego->enemigo);
        tanque_actualizar(juego->jugador, delta_tiempo);
        tanque_actualizar(juego->enemigo, delta_tiempo);
        misil_actualizar(juego->misil_jugador, delta_tiempo);
        misil_actualizar(juego->misil_enemigo, delta_tiempo);

        float jugador_x = tanque_x(juego->jugador), jugador_y = tanque_y(juego->jugador);
        if (hay_colision(juego, jugador_x, jugador_y, enemigo_x_anterior, enemigo_y_anterior))
            tanque_set_posicion(juego->jugador, jugador_x_anterior, jugador_y_anterior);

        float enemigo_x = tanque_x(juego->enemigo), enemigo_y = tanque_y(juego->enemigo);
        if (hay_colision(juego, enemigo_x, enemigo_y, tanque_x(juego->jugador),
                         tanque_y(juego->jugador)))
            tanque_set_posicion(juego->enemigo, enemigo_x_anterior, enemigo_y_anterior);

        float enemigo_phi = tanque_phi(juego->enemigo);
        float distancia_x = tanque_x(juego->jugador) - enemigo_x;
        float distancia_y = tanque_y(juego->jugador) - enemigo_y;
        float angulo_jugador = atan2f(distancia_y, distancia_x);
        float diferencia = angulo_jugador - enemigo_phi;
        while (diferencia > M_PI) diferencia -= 2*M_PI;
        while (diferencia < -M_PI) diferencia += 2*M_PI;

        if (tanque_movimiento(juego->enemigo) == MOV_NINGUNO) {
            if ((rand() % FPS_JUEGO) == 0) {
                if (rand() % 2 == 0) {
                    float duracion = (rand() % 3001) / 1000.0f;
                    if (diferencia > 0)
                        tanque_iniciar_movimiento_tiempo(juego->enemigo, MOV_GIRAR_DER, duracion);
                    else
                        tanque_iniciar_movimiento_tiempo(juego->enemigo, MOV_GIRAR_IZQ, duracion);
                } else {
                    float duracion = ((float)(rand() % 4001) - 1000.0f) / 1000.0f;
                    if (duracion < 0)
                        tanque_iniciar_movimiento_tiempo(juego->enemigo, MOV_ATRAS, -duracion);
                    else
                        tanque_iniciar_movimiento_tiempo(juego->enemigo, MOV_ADELANTE, duracion);
                }
            }
        }

        if (fabs(diferencia) <= 1.0f) {
            float diferencia_torreta = diferencia - tanque_torreta(juego->enemigo);
            while (diferencia_torreta > M_PI) diferencia_torreta -= 2*M_PI;
            while (diferencia_torreta < -M_PI) diferencia_torreta += 2*M_PI;
            if (fabs(diferencia_torreta) > 0.01f) {
                if (diferencia_torreta > 0)
                    tanque_girar_torreta(juego->enemigo, 0.12f * delta_tiempo);
                else
                    tanque_girar_torreta(juego->enemigo, -0.12f * delta_tiempo);
            }
            if (!misil_activo(juego->misil_enemigo) && fabs(diferencia_torreta) < 0.1f &&
                tanque_disparar(juego->enemigo))
                misil_lanzar(juego->misil_enemigo, enemigo_x, enemigo_y,
                             enemigo_phi + tanque_torreta(juego->enemigo));
        } else {
            float torreta = tanque_torreta(juego->enemigo);
            if (fabs(torreta) > 0.01f) {
                if (torreta > 0)
                    tanque_girar_torreta(juego->enemigo, -0.24f * delta_tiempo);
                else
                    tanque_girar_torreta(juego->enemigo, 0.24f * delta_tiempo);
            }
        }

        misil_impacta(juego, juego->misil_jugador, juego->enemigo);
        if (misil_impacta(juego, juego->misil_enemigo, juego->jugador))
            juego->tiempo_impacto = 1.0f;

        if (tanque_vidas(juego->enemigo) <= 0) {
            juego->puntaje += 1000;
            juego->resto_x = tanque_x(juego->enemigo);
            juego->resto_y = tanque_y(juego->enemigo);
            juego->tiempo_resto = 1.5f;
            misil_desactivar(juego->misil_enemigo);
            reaparecer_enemigo(juego);
        }
    }
}

bool   juego_terminado(const juego_t *juego)       { return juego->terminado; }
int    juego_puntaje(const juego_t *juego)          { return juego->puntaje; }
float  juego_tiempo_impacto(const juego_t *juego)   { return juego->tiempo_impacto; }
float  juego_tiempo_resto(const juego_t *juego)     { return juego->tiempo_resto; }
float  juego_resto_x(const juego_t *juego)          { return juego->resto_x; }
float  juego_resto_y(const juego_t *juego)          { return juego->resto_y; }
float  juego_angx(const juego_t *juego)             { return juego->angx; }
float  juego_angz(const juego_t *juego)             { return juego->angz; }

float  juego_jugador_x(const juego_t *juego)        { return tanque_x(juego->jugador); }
float  juego_jugador_y(const juego_t *juego)        { return tanque_y(juego->jugador); }
float  juego_jugador_phi(const juego_t *juego)      { return tanque_phi(juego->jugador); }
int    juego_jugador_vidas(const juego_t *juego)    { return tanque_vidas(juego->jugador); }

bool   juego_enemigo_existe(const juego_t *juego)   { return juego->enemigo != NULL; }
float  juego_enemigo_x(const juego_t *juego)        { return tanque_x(juego->enemigo); }
float  juego_enemigo_y(const juego_t *juego)        { return tanque_y(juego->enemigo); }
float  juego_enemigo_phi(const juego_t *juego)      { return tanque_phi(juego->enemigo); }
float  juego_enemigo_torreta(const juego_t *juego)  { return tanque_torreta(juego->enemigo); }

bool   juego_misil_jugador_activo(const juego_t *juego)  { return misil_activo(juego->misil_jugador); }
float  juego_misil_jugador_x(const juego_t *juego)       { return misil_x(juego->misil_jugador); }
float  juego_misil_jugador_y(const juego_t *juego)       { return misil_y(juego->misil_jugador); }
float  juego_misil_jugador_phi(const juego_t *juego)     { return misil_phi(juego->misil_jugador); }

bool   juego_misil_enemigo_activo(const juego_t *juego)  { return misil_activo(juego->misil_enemigo); }
float  juego_misil_enemigo_x(const juego_t *juego)       { return misil_x(juego->misil_enemigo); }
float  juego_misil_enemigo_y(const juego_t *juego)       { return misil_y(juego->misil_enemigo); }
float  juego_misil_enemigo_phi(const juego_t *juego)     { return misil_phi(juego->misil_enemigo); }

size_t juego_cantidad_obstaculos(const juego_t *juego)              { return juego->cantidad_obstaculos; }
const obstaculo_t *juego_obstaculo(const juego_t *juego, size_t i) { return juego->obstaculos[i]; }

const modelo_t *juego_modelo(const juego_t *juego, const char *nombre) {
    return buscar_modelo(juego->lista_modelos, nombre);
}
