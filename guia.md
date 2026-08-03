# Guía del TP1 — Battle Zone

## Estructura del proyecto

```
main.c          — Loop principal: init SDL, eventos, update, dibujo, cleanup
juego.c/h       — TDA juego: estado (tanques, misiles, obstáculos, score), init, eventos, física, colisiones, IA
dibujo.c/h      — TDA dibujo: render 3D (cámara, mundo, HUD, animaciones) con pila de transformaciones propia
modelo.c/h      — Carga y acceso a modelos 3D (wireframe)
matriz.c/h      — Álgebra de matrices 4×4 (transformaciones 3D)
tanque.c/h      — Lógica del tanque (posición, movimiento, torreta, vidas, enfriamiento)
misil.c/h       — Entidad misil independiente (posición, dirección, vida 2s, velocidad 24 m/s)
obstaculo.c/h   — Obstáculos estáticos (posición + modelo)
stl.c/h         — Lector de archivos STL binario (formato cátedra)
pila.c/h        — Pila genérica (almacena matrices; interna del módulo dibujo)
lista.c/h       — Lista genérica (catálogo de modelos, búsqueda por nombre)
modelos.stl     — Archivo binario con todos los modelos 3D
```

## Archivos

### `main.c` — Loop principal (~60 líneas)

1. **Init**: Inicializa SDL, crea la ventana y el renderizador, llama a `juego_crear()` y a `dibujo_crear(renderizador, juego)`.
2. **Loop**: eventos → `juego_actualizar` → `dibujo_dibujar_mundo` (si no terminó) → `dibujo_dibujar_hud` → `SDL_RenderPresent`. Frame limita a FPS_JUEGO (24); imprime "Perdiendo cuadros" si se pasa.
3. **Cleanup**: `dibujo_destruir`, `juego_destruir`, destroy renderizador/ventana, `SDL_Quit`.

### `juego.c/h` — TDA juego

- Estado: jugador y enemigo (`tanque_t`), dos `misil_t` (jugador y enemigo), 50 obstáculos, puntaje, vidas, timers de animación (impacto, resto), angx/angz (head bob), y la lista `lista_modelos` con todos los modelos STL.
- `juego_crear()`: carga `modelos.stl`, inserta cada modelo en `lista_modelos`, busca los 6 modelos de obstáculos por nombre (`buscar_modelo`), crea 50 obstáculos aleatorios en [-150,150], crea el tanque del jugador en (0,0) mirando +Y, spawnea el enemigo a exactamente 50 unidades girando hasta encontrar una posición sin superposición.
- `juego_modelo(juego, nombre)` — búsqueda de modelos en `lista_modelos` por nombre (requisito de la consigna).
- `juego_actualizar`: timers, head bob, física con colisiones (radio 5), IA enemiga, disparos, colisiones de misiles (radio 3), respawn del enemigo a 50u tras terminar la explosión (+1000 score).
- La lógica de dibujo NO está acá: `juego.h` solo expone accessors de lectura para que `dibujo` renderice.

### `dibujo.c/h` — Render (presentación)

- Tipo opaco `dibujo_t`: guarda el `SDL_Renderer*` y una `pila_t*` de transformaciones propia (con la identidad en el fondo).
- `dibujo_crear(renderizador, juego)` además obtiene con `juego_modelo` todas las referencias a los modelos que se van a dibujar.
- `dibujo_dibujar_mundo`: arma la cámara `mper × Mz(π/2+angz) × My(π/2-angx) × Mz(-phi) × Mt(-x,-y,-3)`, renderiza fondo, obstáculos, enemigo, misiles y la explosión.
- `dibujo_dibujar_hud`: cámara HUD, mira (`+` si enemigo a <0.15 rad, `-` si no), GAME OVER, impacto (#), vidas (*), score, indicador de dirección y brújula.
- Helpers internos: `renderizar_modelo` (transforma y proyecta), `renderizar_texto` (letras), `renderizar_digitos` (score, centrado o a derecha), `renderizar_explosion` (6 piezas con tiro oblicuo vx=5, vz=10, g=9.81).
- La pila se apila/desapila dentro del módulo; `main` y `juego` no la conocen.

### `modelo.c/h` — Modelos 3D

- Tipo opaco `modelo_t`. Se crea con `modelo_crear(nombre, coords, ncoords, lineas, nlineas)`.
- **Copia** los arrays (el caller puede hacer free inmediato después).
- Accessors: `modelo_coords`, `modelo_lineas`, `modelo_ncoords`, `modelo_nlineas`, `modelo_nombre`.
- Las coordenadas son (x,y,z) contiguas en un float array. Las líneas son pares de índices.

### `matriz.c/h` — Álgebra de matrices

- Tipo opaco `matriz_t` con `float *m` (row-major), `filas`, `columnas`.
- `_matriz_crear(n,m)` — allocator interno (prefijo _ indica uso interno).
- Fábricas de matrices 4×4:
  - `identidad(n)` — diagonal de 1s
  - `mz(θ)` — rotación en Z (plano XY)
  - `my(θ)` — rotación en Y (plano XZ)
  - `mc(θ)` — alias de `mz`
  - `mn(n)` — "null space" (reflection)
  - `mt(v)` — traslación por vector (x,y,z)
  - `mper(d)` — perspectiva: fila inferior [0,0,-1/d,0]
- `matriz_multiplicar(a,b)` — producto matricial, devuelve nueva matriz
- `matriz_aplicar(transform, puntos)` — aplica transformación 4×4 a puntos 3D (agrega w=1, divide por w homogéneo). Devuelve matriz (n,3) con (x', y', depth) donde depth = w homogénea original (sin dividir).

### `tanque.c/h` — Tanque

- Tipo opaco `tanque_t` con: posición (x,y), heading (phi), torreta, vidas, enfriamiento.
- Movimiento: `tanque_iniciar_movimiento` (timer 0.5s) o `_tiempo` (duración variable). Comandar el mismo movimiento reinicia el timer (consigna: cada comando = 0.5s).
- `tanque_actualizar`: mueve 7m/s en dirección del heading, gira 0.36 rad/s, decrementa enfriamiento.
- `tanque_disparar`: true si el enfriamiento está en 0; lo setea en 2s. NO crea el misil (eso lo hace `juego`).
- `tanque_puede_disparar`: enfriamiento ≤ 0.
- Torreta limitada a ±1 rad.
- `crear_tanque_enemigo`: valida que no haya obstáculo a < 5 unidades; retorna NULL si hay superposición.

### `misil.c/h` — Misil

- Tipo opaco `misil_t`: posición (x,y), dirección (phi), activo, tiempo de vida.
- `misil_lanzar(m, x, y, phi)` lo activa desde una posición hacia una dirección; `misil_desactivar` lo apaga (colisión).
- `misil_actualizar`: avanza 24 m/s, vida 2s; al agotarse se desactiva solo.
- `juego` posee dos: uno del jugador y uno del enemigo.

### `obstaculo.c/h`

- Tipo opaco. Solo almacena posición (x,y), rotación (phi), y puntero al modelo (no owned).

### `stl.c/h` — Lector STL binario (formato cátedra)

- Formato: header "STL" + versión 3 + offset 25, luego formato (unidades, maxlong), luego modelos.
- Cada modelo: etiqueta (maxlong chars), ncoords, floats [3*ncoords], nlineas, int32s [2*nlineas].
- Funciones: `leer_encabezado_stl`, `leer_formato_STL`, `leer_modelo_3d` (allocatea coords y lineas, caller debe free).
- Unidades: MM, CM, M, IN, FT, MILS.

### `pila.c/h` — Pila genérica

- Almacena `void*`. Usada para el stack de matrices de transformación.
- `pila_apilar`, `pila_tope`, `pila_desapilar`.
- Interna del módulo `dibujo`: `main` y `juego` no la manejan.
- Cleanup: destruir la identidad inicial con `matriz_destruir(pila_desapilar(stack))` antes de `pila_destruir(stack)`.

## Convenciones del mundo

| Concepto | Valor |
|----------|-------|
| Mundo | x,y ∈ [-150, 150] |
| Player | (0,0), heading π/2 (+Y), 4 vidas |
| Obstáculos | 50 aleatorios (CUBO1-3, PIRAMIDE1-3) |
| Enemigo | Spawnea a 50u exactas, 1 vida |
| Score | +1000 por kill |
| FPS | 24; si no alcanza imprime "Perdiendo cuadros" |

## Colores

- Fondo: horizonte gris, montañas gris oscuro, luna amarillo claro
- Obstáculos: verde
- Enemigo (hull/torreta/radar): blanco
- Misil player: blanco, Misil enemigo: rojo
- Explosión enemiga: rojo
- Hit player (#): rojo
- Mira: rojo; HUD/vidas/score: blanco

## Gotchas

- Tipos opacos — usar accessors, nunca dereferenciar structs
- `modelo_crear` copia los arrays; se puede hacer free inmediato después
- `matriz_crear_mc` ≡ `matriz_crear_mz`
- `_matriz_crear` es allocator interno (usar con cuidado)
- El modelo STL se llama "MONTANA" (sin S), no "MONTANAS"
- `crear_tanque_enemigo` retorna NULL si el spawn se superpone con un obstáculo (< 5u)
