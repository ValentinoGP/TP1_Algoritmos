# Guía TP1 — main.c

Esqueleto de un juego *Battle Zone* (tanques en 3D alámbrico). Hay que implementar el juego real dentro de las secciones marcadas.

## Init (líneas 27–64)

Los modelos 3D ya se cargan desde `modelos.stl` en `lista_modelos`. Hay que:

- **Recorrer** `lista_modelos` y según el nombre de cada modelo (`modelo_nombre(m->modelo)`), crear:
  - **Tanques** con `tanque_crear(x, y, phi, vidas)` — uno para el jugador, uno o más para la IA.
  - **Obstáculos** con `obstaculo_crear(x, y, phi, modelo)`.
- Guardar punteros a los tanques/obstáculos creados. Se pueden declarar variables locales o arrays estáticos.

## Evento (líneas 72–96)

El teclado controla al tanque del jugador. Ya hay un `switch` de ejemplo. Cambiarlo para:

| Tecla | Acción |
|-------|--------|
| Flechas | `tanque_girar(t, delta)` / `tanque_mover(t, delta)` |
| Otra tecla (A/D) | `tanque_girar_torreta(t, delta)` |
| Espacio | `tanque_disparar(t)` si `tanque_puede_disparar(t)` |

## Draw (líneas 105–108)

Acá va la lógica por frame y el renderizado:

1. **Actualizar estado**: `tanque_actualizar(t, dt)` para cada tanque, movimiento de misiles, colisiones, IA enemiga, etc.
2. **Renderizar**: para cada tanque/obstáculo, proyectar sus coordenadas 3D a 2D (rotación + traslación) y dibujar las líneas con `SDL_RenderDrawLine`. Usar `modelo_coords(m)` y `modelo_lineas(m)`.
3. El `continue` del evento hace que update y render solo se ejecuten cuando **no hay evento pendiente**. No se puede intercalar lógica de juego dentro del manejador de eventos.

## Cleanup (líneas 123–133)

Ya libera `lista_modelos`. Agregar `tanque_destruir` y `obstaculo_destruir` para los que se hayan creado en Init.

---

En resumen: el `main.c` es el **orquestador**. Usar las funciones de `tanque.h`, `obstaculo.h` y `modelo.h` para armar el juego.
