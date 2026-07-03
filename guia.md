# Guía del TP1 — Battle Zone

## Estructura del proyecto

```
main.c          — Loop principal: init, eventos, física, render, cleanup
modelo.c/h      — Carga y acceso a modelos 3D (wireframe)
matriz.c/h      — Álgebra de matrices 4×4 (transformaciones 3D)
tanque.c/h      — Lógica del tanque (posición, movimiento, misil, colisiones)
obstaculo.c/h   — Obstáculos estáticos (posición + modelo)
stl.c/h         — Lector de archivos STL binario (formato cátedra)
pila.c/h        — Pila genérica (almacena matrices)
lista.c/h       — Lista genérica (no se usa en main.c)
modelos.stl     — Archivo binario con todos los modelos 3D
```

## Archivos

### `main.c` — Loop principal (~770 líneas)

1. **Init** (líneas 86–201): Carga `modelos.stl`, crea lista enlazada de modelos, asigna cada modelo por nombre a su variable. Crea 50 obstáculos aleatorios en [-150,150]. Crea el tanque del jugador en (0,0) mirando +Y. Spawnea el enemigo a exactamente 50 unidades, girando hasta encontrar una posición sin superposición. Inicializa pila con matriz identidad.

2. **Eventos** (líneas 213–234): SDL_KEYDOWN → `tanque_iniciar_movimiento` (UP/DOWN/LEFT/RIGHT) o `tanque_disparar` (SPACE). Timer 0.5s por movimiento.

3. **Draw** (líneas 241–741): Por frame:
   - Actualiza timers (hit, resto, game_over)
   - Head bob: angx/angz varían ±0.00001 al moverse, decaen ×0.92
   - Guarda posición anterior, actualiza física, revierte si hay colisión (radio 5)
   - IA enemiga: movimiento aleatorio (1/FPS prob), rotación hacia el jugador, torreta apunta si |diff|≤1 rad, dispara si apunta, evita obstáculos
   - Construye matriz de cámara: `mper × Mz(π/2+angz) × My(π/2-angx) × Mz(-phi) × Mt(-x,-y,-3)`
   - Renderiza fondo (horizonte gris, montañas gris oscuro, luna amarilla en (0,500,-50))
   - Renderiza obstáculos (verde), enemigo (blanco, radar rotatorio), misiles
   - Colisiones de misiles (radio 3) con obstáculos y tanque opuesto
   - Respawn enemigo a 50u al morir (+1000 score)
   - Animación de destrucción: 6 piezas con tiro oblicuo (vx=5, vz=10, g=9.81)
   - Crosshair 2D, brújula, HUD 3D (vidas arriba-izq, score arriba-der)
   - Indicador dirección enemiga: IZQUIERDA/DERECHA/ATRAS fuera de FOV
   - Game over: texto rojo + score blanco

4. **Cleanup** (líneas 756–772): Libera modelos, obstáculos, tanques, pila.

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

- Tipo opaco `tanque_t` con: posición (x,y), heading (phi), torreta, vidas, enfriamiento, misil.
- Movimiento: `tanque_iniciar_movimiento` (timer 0.5s) o `_tiempo` (duración variable).
- `tanque_actualizar`: mueve 7m/s en dirección del heading, gira 0.36 rad/s. Misil 24m/s, lifetime 2s.
- `tanque_puede_disparar`: enfriamiento ≤ 0 y sin misil activo. Cooldown 2s.
- Torreta limitada a ±1 rad.
- `crear_tanque_enemigo`: valida que no haya obstáculo a < 5 unidades; retorna NULL si hay superposición.

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
- Crosshair/HUD/vidas/score: blanco

## Gotchas

- Tipos opacos — usar accessors, nunca dereferenciar structs
- `modelo_crear` copia los arrays; se puede hacer free inmediato después
- `matriz_crear_mc` ≡ `matriz_crear_mz`
- `_matriz_crear` es allocator interno (usar con cuidado)
- El modelo STL se llama "MONTANA" (sin S), no "MONTANAS"
- `crear_tanque_enemigo` retorna NULL si el spawn se superpone con un obstáculo (< 5u)
