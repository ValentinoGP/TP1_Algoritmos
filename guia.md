# Guía TP1 — main.c

Esqueleto de *Battle Zone* (tanques 3D alámbrico, first-person shooter).

## Init

Los modelos 3D se cargan desde `modelos.stl` en `lista_modelos`. En Init se hace:

1. **Identificar modelos** por nombre: `TANQUE`, `TORRETA`, `RADAR`, `MISIL`, `HORIZONTE`, `MONTANAS`, `LUNA`, y los 6 de obstáculos (`CUBO1`–`CUBO3`, `PIRAMIDE1`–`PIRAMIDE3`).
2. **Crear 50 obstáculos** en `x,y ∈ [-150, 150]` con rotación aleatoria, eligiendo al azar entre los 6 modelos de obstáculo.
3. **Crear jugador** en `(0, 0)` con `phi = π/2` (mirando +Y) y 4 vidas.
4. **Crear enemigo** en posición aleatoria a 50 unidades del jugador, que no se superponga con ningún obstáculo (distancia ≥ 5).
5. **Inicializar la pila de transformaciones** (stack) con la matriz identidad.

## Evento

El teclado controla al jugador. No hay A/D para torreta (el enunciado dice que el jugador **no** mueve la torreta).

| Tecla | Acción |
|-------|--------|
| Up | Inicia movimiento hacia adelante (7 m/s, 0.5 s) |
| Down | Inicia movimiento hacia atrás (7 m/s, 0.5 s) |
| Left | Inicia giro a izquierda (0.36 rad/s, 0.5 s) |
| Right | Inicia giro a derecha (0.36 rad/s, 0.5 s) |
| Space | `tanque_disparar(jugador)` si `tanque_puede_disparar(jugador)` |

Si se vuelve a presionar la misma tecla, se reinicia el timer de 0.5 s.

Los eventos se drenan con un `while (SDL_PollEvent(...))` anidado para no saltear el renderizado.

## Draw

Lógica por frame y renderizado 3D:

1. **Calcular dt** entre cuadros (clamped a 0.1s).
2. **Head bob**: si el jugador se mueve, `angx`/`angz` varían ±0.00001 aleatorio y decaen ×0.92 por frame.
3. **Actualizar estado**: `tanque_actualizar` para ambos tanques (movimiento, misiles, enfriamiento).
4. **Colisiones**: se guarda la posición anterior, se actualiza, y si la nueva posición está a < 5 de un obstáculo o del otro tanque, se vuelve atrás.
5. **IA enemiga**: calcula ángulo hacia el jugador. Si está dentro de 1 rad del frente, la torreta lo apunta (velocidad 3 × dt). Si apunta a < 0.1 rad, dispara. Si está fuera del FOV, torreta vuelve a 0. Movimiento aleatorio cada 2–5 segundos.
6. **Construir matriz de cámara** (multiplicación secuencial, un solo push):
   ```
   cam = I × MPER × Mz(π/2 + angz) × My(π/2 - angx) × Mz(-phi) × Mt(-x, -y, -3)
   ```
7. **Renderizar con `renderizar_modelo`**:
   - Crea `Mt(x,y,z)` y `Mz(rot)`, multiplica: `obj = Mt × Mz`
   - Combina con cámara: `final = cam × obj`, apila
   - Aplica `matriz_aplicar(final, pts_3d)` → (sx, sy, depth)
   - Dibuja líneas si ambos extremos tienen depth ≥ 1 (delante de cámara)
   - Desapila todo
   - Se renderizan: obstáculos (blanco), enemigo (hull rojo, torreta naranja, radar amarillo), misiles (verde jugador, rojo enemigo), fondo (HORIZONTE, MONTANAS, LUNA)
8. **Colisión de misiles**: cada misil checkea distancia < 3 contra obstáculos y contra el otro tanque. Si impacta, el misil desaparece. Si impacta al tanque, le saca una vida.
9. **Respawn del enemigo**: si el enemigo se queda sin vidas, se destruye y aparece uno nuevo a 50 u. del jugador (3 vidas).
10. **Crosshair 2D**: dos líneas blancas en el centro de la pantalla.

## Cleanup

Liberar `lista_modelos` (recorrer nodos, destruir modelo y nodo), obstáculos (array), tanques (jugador y enemigo), y pila de transformaciones.

Recordar: `valgrind --leak-check=full ./battlezone` debe dar 0 pérdidas definitivas/indirectas/posibles.

## TODO (lo que falta implementar)

1. **Animaciones**: `#` para hit al jugador, `RESTO1`/`RESTO2` para destrucción de enemigo
2. **HUD**: vidas (`*`), puntaje, indicador de dirección
3. **Score** y game over
