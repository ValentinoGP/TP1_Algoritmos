#include "dibujo.h"
#include "modelo.h"
#include "obstaculo.h"
#include "matriz.h"
#include "pila.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VENTANA_ANCHO 1024
#define VENTANA_ALTO 768
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
    const modelo_t *estrella;
    const modelo_t *gato;
    const modelo_t *mas;
    const modelo_t *menos;
    const modelo_t *resto1;
    const modelo_t *resto2;
    const modelo_t *letras[26];
    const modelo_t *digitos[10];
} modelos_t;

struct dibujo {
    SDL_Renderer *renderizador;
    pila_t *pila;
    modelos_t modelos;
};

static void obtener_modelos(modelos_t *modelos, const juego_t *juego) {
    modelos->horizonte = juego_modelo(juego, "HORIZONTE");
    modelos->montana   = juego_modelo(juego, "MONTANA");
    modelos->luna      = juego_modelo(juego, "LUNA");
    modelos->tanque    = juego_modelo(juego, "TANQUE");
    modelos->torreta   = juego_modelo(juego, "TORRETA");
    modelos->radar     = juego_modelo(juego, "RADAR");
    modelos->misil     = juego_modelo(juego, "MISIL");
    modelos->estrella  = juego_modelo(juego, "*");
    modelos->gato      = juego_modelo(juego, "#");
    modelos->mas       = juego_modelo(juego, "+");
    modelos->menos     = juego_modelo(juego, "-");
    modelos->resto1    = juego_modelo(juego, "RESTO1");
    modelos->resto2    = juego_modelo(juego, "RESTO2");
    for (int i = 0; i < 26; i++) {
        char nombre[2] = {'A' + i, '\0'};
        modelos->letras[i] = juego_modelo(juego, nombre);
    }
    for (int i = 0; i < 10; i++) {
        char nombre[2] = {'0' + i, '\0'};
        modelos->digitos[i] = juego_modelo(juego, nombre);
    }
}

dibujo_t *dibujo_crear(SDL_Renderer *renderizador, const juego_t *juego) {
    dibujo_t *dibujo = malloc(sizeof(dibujo_t));
    if (!dibujo) return NULL;

    dibujo->pila = pila_crear();
    if (!dibujo->pila) {
        free(dibujo);
        return NULL;
    }
    matriz_t *identidad = matriz_crear_identidad(4);
    if (!identidad) {
        pila_destruir(dibujo->pila, NULL);
        free(dibujo);
        return NULL;
    }
    pila_apilar(dibujo->pila, identidad);

    dibujo->renderizador = renderizador;
    obtener_modelos(&dibujo->modelos, juego);
    return dibujo;
}

void dibujo_destruir(dibujo_t *dibujo) {
    if (!dibujo) return;
    matriz_destruir(pila_desapilar(dibujo->pila));
    pila_destruir(dibujo->pila, NULL);
    free(dibujo);
}

static void renderizar_modelo(dibujo_t *dibujo, const modelo_t *modelo,
                              float x, float y, float z, float rotacion, size_t limite_lineas) {
    if (!modelo) return;

    const float *camara = matriz_datos(pila_ver_tope(dibujo->pila));

    float cos_rot = cosf(rotacion), sen_rot = sinf(rotacion);
    float matriz_objeto[16] = {
        cos_rot, -sen_rot, 0, x,
        sen_rot,  cos_rot, 0, y,
        0,  0, 1, z,
        0,  0, 0, 1
    };

    float transformacion[16];
    for (int fila = 0; fila < 4; fila++)
        for (int columna = 0; columna < 4; columna++) {
            float suma = 0;
            for (int k = 0; k < 4; k++)
                suma += camara[fila*4+k] * matriz_objeto[k*4+columna];
            transformacion[fila*4+columna] = suma;
        }

    const float *coordenadas = modelo_coords(modelo);
    const size_t *lineas = modelo_lineas(modelo);
    size_t cantidad_lineas = modelo_nlineas(modelo);
    if (limite_lineas == 0 || limite_lineas > cantidad_lineas) limite_lineas = cantidad_lineas;

    for (size_t i = 0; i < 2 * limite_lineas; i += 2) {
        size_t indice_inicial = lineas[i], indice_final = lineas[i+1];

        float vertice_a[4] = {coordenadas[3*indice_inicial], coordenadas[3*indice_inicial+1],
                              coordenadas[3*indice_inicial+2], 1};
        float proyectado_a[4];
        for (int fila = 0; fila < 4; fila++) {
            float suma = 0;
            for (int k = 0; k < 4; k++) suma += transformacion[fila*4+k] * vertice_a[k];
            proyectado_a[fila] = suma;
        }
        if (proyectado_a[3] == 0) continue;
        float profundidad_a = proyectado_a[3];
        float x_a = proyectado_a[0] / proyectado_a[3];
        float y_a = proyectado_a[1] / proyectado_a[3];

        float vertice_b[4] = {coordenadas[3*indice_final], coordenadas[3*indice_final+1],
                              coordenadas[3*indice_final+2], 1};
        float proyectado_b[4];
        for (int fila = 0; fila < 4; fila++) {
            float suma = 0;
            for (int k = 0; k < 4; k++) suma += transformacion[fila*4+k] * vertice_b[k];
            proyectado_b[fila] = suma;
        }
        if (proyectado_b[3] == 0) continue;
        float profundidad_b = proyectado_b[3];
        float x_b = proyectado_b[0] / proyectado_b[3];
        float y_b = proyectado_b[1] / proyectado_b[3];

        if (profundidad_a >= 1.0f && profundidad_b >= 1.0f) {
            int pantalla_x_a = (int)(x_a * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
            int pantalla_y_a = (int)(VENTANA_ALTO/2 - y_a * VENTANA_ALTO/2);
            int pantalla_x_b = (int)(x_b * VENTANA_ALTO/2 + VENTANA_ANCHO/2);
            int pantalla_y_b = (int)(VENTANA_ALTO/2 - y_b * VENTANA_ALTO/2);
            SDL_RenderDrawLine(dibujo->renderizador, pantalla_x_a, pantalla_y_a,
                               pantalla_x_b, pantalla_y_b);
        }
    }
}

static void transformar_tope(pila_t *pila, matriz_t *nueva) {
    matriz_t *tope = pila_ver_tope(pila);
    matriz_t *aux = matriz_multiplicar(tope, nueva);
    matriz_destruir(tope);
    matriz_destruir(nueva);
    pila_desapilar(pila);
    pila_apilar(pila, aux);
}

static void construir_camara(dibujo_t *dibujo, const juego_t *juego) {
    float jugador_x = juego_jugador_x(juego);
    float jugador_y = juego_jugador_y(juego);
    float jugador_phi = juego_jugador_phi(juego);
    float angx = juego_angx(juego);
    float angz = juego_angz(juego);

    if (angx <= 0.01f) angx = 0;

    pila_apilar(dibujo->pila, matriz_crear_identidad(4));
    transformar_tope(dibujo->pila, matriz_crear_mper(1.8f));
    transformar_tope(dibujo->pila, matriz_crear_mz(M_PI / 2 + angz));
    transformar_tope(dibujo->pila, matriz_crear_my(M_PI / 2 - angx));
    transformar_tope(dibujo->pila, matriz_crear_mz(-jugador_phi));
    float vector[3] = {-jugador_x, -jugador_y, -3.0f};
    transformar_tope(dibujo->pila, matriz_crear_mt(vector));
}

static void construir_camara_hud(dibujo_t *dibujo) {
    pila_apilar(dibujo->pila, matriz_crear_identidad(4));
    transformar_tope(dibujo->pila, matriz_crear_mper(1.8f));
    float desplazamiento_z[3] = {0, 0, -12};
    transformar_tope(dibujo->pila, matriz_crear_mt(desplazamiento_z));
}

static void renderizar_explosion(dibujo_t *dibujo, const juego_t *juego) {
    if (juego_tiempo_resto(juego) <= 0) return;

    const modelo_t *modelo_resto1 = dibujo->modelos.resto1;
    if (!modelo_resto1) return;

    SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0x00, 0x00, 0x00);
    float tiempo = 1.5f - juego_tiempo_resto(juego);
    float distancia_x = 5.0f * tiempo;
    float altura = 10.0f * tiempo - 0.5f * 9.81f * tiempo * tiempo;
    if (altura < 0) altura = 0;

    const modelo_t *piezas[6];
    piezas[0] = dibujo->modelos.torreta;
    piezas[1] = dibujo->modelos.radar;
    piezas[2] = modelo_resto1;
    piezas[3] = dibujo->modelos.resto2;
    piezas[4] = modelo_resto1;
    piezas[5] = dibujo->modelos.resto2;

    float origen_x = juego_resto_x(juego);
    float origen_y = juego_resto_y(juego);

    for (int i = 0; i < 6; i++) {
        if (!piezas[i]) continue;
        float angulo = i * (2 * M_PI / 6);
        float pieza_x = origen_x + distancia_x * cosf(angulo);
        float pieza_y = origen_y + distancia_x * sinf(angulo);
        renderizar_modelo(dibujo, piezas[i],
                          pieza_x, pieza_y, altura, angulo, 0);
    }
}

static void renderizar_texto(dibujo_t *dibujo, const char *texto, float y, float separacion) {
    int longitud = strlen(texto);
    float inicio_x = -longitud * separacion / 2;
    for (int i = 0; i < longitud; i++) {
        int indice = texto[i] - 'A';
        const modelo_t *letra = (indice >= 0 && indice < 26) ? dibujo->modelos.letras[indice] : NULL;
        if (letra)
            renderizar_modelo(dibujo, letra,
                              inicio_x + i * separacion, y, 0, 0, 0);
    }
}

static void renderizar_digitos(dibujo_t *dibujo, int valor, float pos_x, float y,
                               float separacion, bool derecha) {
    char texto[10];
    int n = 0;
    if (valor == 0) {
        texto[n++] = '0';
    } else {
        while (valor > 0) {
            texto[n++] = '0' + valor % 10;
            valor /= 10;
        }
    }
    float x = derecha ? pos_x - (n - 1) * separacion : -n * separacion / 2;
    for (int i = n - 1; i >= 0; i--) {
        int indice = texto[i] - '0';
        const modelo_t *digito = (indice >= 0 && indice < 10) ? dibujo->modelos.digitos[indice] : NULL;
        if (digito)
            renderizar_modelo(dibujo, digito, x, y, 0, 0, 0);
        x += separacion;
    }
}

static float diferencia_enemigo(const juego_t *juego) {
    if (!juego_enemigo_existe(juego)) return 0;
    float angulo_enemigo = atan2f(juego_enemigo_y(juego) - juego_jugador_y(juego),
                                  juego_enemigo_x(juego) - juego_jugador_x(juego));
    float diferencia = angulo_enemigo - juego_jugador_phi(juego);
    while (diferencia > M_PI) diferencia -= 2*M_PI;
    while (diferencia < -M_PI) diferencia += 2*M_PI;
    return diferencia;
}

void dibujo_dibujar_mundo(dibujo_t *dibujo, const juego_t *juego) {
    construir_camara(dibujo, juego);

    if (dibujo->modelos.horizonte) {
        SDL_SetRenderDrawColor(dibujo->renderizador, 0x80, 0x80, 0x80, 0x00);
        renderizar_modelo(dibujo, dibujo->modelos.horizonte,
                          0, 0, 0, 0, 0);
    }
    if (dibujo->modelos.montana) {
        SDL_SetRenderDrawColor(dibujo->renderizador, 0x40, 0x40, 0x40, 0x00);
        renderizar_modelo(dibujo, dibujo->modelos.montana,
                          0, 0, 0, 0, 0);
    }
    if (dibujo->modelos.luna) {
        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xCC, 0x00);
        renderizar_modelo(dibujo, dibujo->modelos.luna,
                          0, 500, -50, 0, 0);
    }

    SDL_SetRenderDrawColor(dibujo->renderizador, 0x00, 0xFF, 0x00, 0x00);
    for (size_t i = 0; i < juego_cantidad_obstaculos(juego); i++) {
        const obstaculo_t *obstaculo = juego_obstaculo(juego, i);
        renderizar_modelo(dibujo, obstaculo_modelo(obstaculo),
                          obstaculo_x(obstaculo), obstaculo_y(obstaculo),
                          0.0f, obstaculo_phi(obstaculo), 0);
    }

    if (juego_enemigo_existe(juego)) {
        float enemigo_x = juego_enemigo_x(juego);
        float enemigo_y = juego_enemigo_y(juego);
        float enemigo_phi = juego_enemigo_phi(juego);
        float enemigo_torreta = juego_enemigo_torreta(juego);

        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xFF, 0x00);
        renderizar_modelo(dibujo, dibujo->modelos.tanque,
                          enemigo_x, enemigo_y, 0.0f, enemigo_phi, 0);
        renderizar_modelo(dibujo, dibujo->modelos.torreta,
                          enemigo_x, enemigo_y, 3.0f, enemigo_phi + enemigo_torreta, 0);

        if (dibujo->modelos.radar) {
            float rotacion_radar = SDL_GetTicks() / 500.0f;
            float radar_x = enemigo_x + (-1.5f) * cosf(enemigo_phi + enemigo_torreta);
            float radar_y = enemigo_y + (-1.5f) * sinf(enemigo_phi + enemigo_torreta);
            SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xFF, 0x00);
            renderizar_modelo(dibujo, dibujo->modelos.radar,
                              radar_x, radar_y, 3.5f,
                              enemigo_phi + enemigo_torreta + rotacion_radar, 0);
        }
    }

    if (dibujo->modelos.misil && juego_misil_jugador_activo(juego)) {
        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xFF, 0x00);
        renderizar_modelo(dibujo, dibujo->modelos.misil,
                          juego_misil_jugador_x(juego),
                          juego_misil_jugador_y(juego),
                          2.5f,
                          juego_misil_jugador_phi(juego), 0);
    }
    if (dibujo->modelos.misil && juego_misil_enemigo_activo(juego)) {
        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0x00, 0x00, 0x00);
        renderizar_modelo(dibujo, dibujo->modelos.misil,
                          juego_misil_enemigo_x(juego),
                          juego_misil_enemigo_y(juego),
                          2.5f,
                          juego_misil_enemigo_phi(juego), 0);
    }

    renderizar_explosion(dibujo, juego);

    matriz_destruir(pila_desapilar(dibujo->pila));
}

void dibujo_dibujar_hud(dibujo_t *dibujo, const juego_t *juego) {
    float diferencia = diferencia_enemigo(juego);

    construir_camara_hud(dibujo);

    if (!juego_terminado(juego)) {
        const modelo_t *mira = (fabs(diferencia) < 0.15f) ? dibujo->modelos.mas : dibujo->modelos.menos;
        if (mira) {
            SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0x00, 0x00, 0x00);
            renderizar_modelo(dibujo, mira, 0, 0, 0, 0, 0);
        }
    }

    if (juego_terminado(juego)) {
        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0x00, 0x00, 0x00);
        renderizar_texto(dibujo, "GAME OVER", 1.04f, 0.9f);
        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xFF, 0x00);
        renderizar_digitos(dibujo, juego_puntaje(juego), 0, -0.39f, 1.17f, false);
    } else {
        if (juego_tiempo_impacto(juego) > 0 && dibujo->modelos.gato) {
            SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0x00, 0x00, 0x00);
            size_t lineas_totales = modelo_nlineas(dibujo->modelos.gato);
            size_t lineas_a_dibujar = (size_t)((1.0f - juego_tiempo_impacto(juego)) * lineas_totales);
            if (lineas_a_dibujar > lineas_totales) lineas_a_dibujar = lineas_totales;
            renderizar_modelo(dibujo, dibujo->modelos.gato,
                              0, 0, 0, 0, lineas_a_dibujar);
        }

        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xFF, 0x00);
        for (int i = 0; i < juego_jugador_vidas(juego) - 1 && dibujo->modelos.estrella; i++)
            renderizar_modelo(dibujo, dibujo->modelos.estrella,
                              -7.8f + i * 1.95f, 4.92f, 0, 0, 0);

        renderizar_digitos(dibujo, juego_puntaje(juego), 5.82f, 4.92f, 1.17f, true);

        if (fabs(diferencia) > 1.0f && juego_enemigo_existe(juego)) {
            const char *direccion = "ATRAS";
            if (diferencia > 1.0f && diferencia <= 2.44f)
                direccion = "IZQUIERDA";
            else if (diferencia < -1.0f && diferencia >= -2.44f)
                direccion = "DERECHA";
            SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xFF, 0x00);
            renderizar_texto(dibujo, direccion, 0.65f, 0.9f);
        }

        float jugador_phi = juego_jugador_phi(juego);
        int centro_brujula_x = VENTANA_ANCHO / 2;
        int centro_brujula_y = VENTANA_ALTO - 50;
        int radio_brujula = 18;
        SDL_SetRenderDrawColor(dibujo->renderizador, 0x80, 0x80, 0x80, 0x00);
        SDL_RenderDrawLine(dibujo->renderizador, centro_brujula_x - radio_brujula, centro_brujula_y,
                           centro_brujula_x + radio_brujula, centro_brujula_y);
        SDL_RenderDrawLine(dibujo->renderizador, centro_brujula_x, centro_brujula_y - radio_brujula,
                           centro_brujula_x, centro_brujula_y + radio_brujula);
        SDL_SetRenderDrawColor(dibujo->renderizador, 0xFF, 0xFF, 0xFF, 0x00);
        int punta_x = centro_brujula_x + (int)(cosf(jugador_phi) * radio_brujula);
        int punta_y = centro_brujula_y - (int)(sinf(jugador_phi) * radio_brujula);
        SDL_RenderDrawLine(dibujo->renderizador, centro_brujula_x, centro_brujula_y, punta_x, punta_y);
    }

    matriz_destruir(pila_desapilar(dibujo->pila));
}
