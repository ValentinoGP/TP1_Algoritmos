# AGENTS.md — TP1_algoritmos

Battle Zone (FIUBA Algoritmos) — clon del Atari con SDL2 y motor 3D propio.

## Política de código

Este repo no busca código perfecto ni sobreingeniería. Busca código **realista de un estudiante que está aprendiendo TDAs**: funcional, que compile sin warnings, que ande, pero sin vueltas raras, sin macros rebuscadas, sin patrones que un pibe de algoritmos no escribiría. Comentarios en español y sencillos.

## Build & run

```sh
make          # produce ./battlezone
./battlezone  # modelos.stl must be in CWD
```

Compile **without warnings**. On macOS: `valgrind --leak-check=full ./battlezone` (no `suppressions.supp` in repo).

## Student sections only

`main.c` has four `BEGIN código del alumno` / `END código del alumno` blocks. Everything else is framework.

| Section | Lines | Purpose |
|---------|-------|---------|
| Init    | 84–183 | Cargar STL, crear obstáculos, tanques, pila |
| Event   | 194–215 | Input del jugador |
| Draw    | 222–474 | Física, IA, colisiones, render 3D |
| Cleanup | 488–503 | Liberar memoria |

Render 3D via `renderizar_modelo` (helper static, líneas 19–59, fuera del alumno).

## Implementado vs. pendiente

**Hecho:** carga de modelos, obstáculos, tanques, movimiento del jugador (timer 0.5s), misiles, cámara 3D, wireframe, crosshair, fondo, colisiones (radio 5 mov, radio 3 misiles), muerte y respawn del enemigo, IA enemiga (torreta trackea dentro de 1 rad, dispara si apunta a < 0.1 rad, movimiento aleatorio).

**Falta:**
- Animaciones: `#` para hit al jugador, `RESTO1`/`RESTO2` para muerte del enemigo
- HUD: vidas (`*`), puntaje, indicador de dirección
- Score / game over

## Referencia rápida

- **Mundo 2D:** x,y ∈ `[-150, 150]`. Player empieza en `(0,0)` mirando `π/2` (+Y). 50 obstáculos aleatorios. Enemigo spawnea a 50 u. del player sin superponerse a obstáculos.
- **Modelos:** `CUBO1-3`/`PIRAMIDE1-3` (obstáculos), `TANQUE`/`TORRETA`/`RADAR` (tanque), `MISIL`, `HORIZONTE`/`MONTANAS`/`LUNA` (fondo), `A-Z`/`0-9`/` ` (HUD), `*`/`-`/`+`/`#` (iconos), `RESTO1`/`RESTO2` (debris).
- **Teclas:** Up/Down mover, Left/Right girar, Space disparar (cooldown 2s). Torreta del player fija.
- **Cámara:** `cam = I × MPER × Mz(π/2+angz) × My(π/2-angx) × Mz(-phi) × Mt(-x, -y, -3)` con head-bob (`angx`/`angz` decaen ×0.92 por frame).

## Modules

`matriz` (EJ1), `matriz_aplicar` (EJ3, devuelve 3 cols, depth en col 2 = w'), `pila` (Cátedra), `lista` (Cátedra, no se usa en main.c). `_matriz_crear` es el allocator interno, usado en `renderizar_modelo`.

## Gotchas

- `tanque.c:5–7`: `#ifndef M_PI #define M_PI 3.14159…` con guarda.
- `modelo_crear` copia los arrays; el caller puede `free` coords/lineas apenas crea el modelo.
- Todos los tipos son opacos — usar accessors.
- `guia.md` tiene instrucciones viejas; los line numbers exactos están arriba.
- Proyección en `renderizar_modelo`: `sx = (int)(x' * H/2 + W/2)`, `sy = (int)(H/2 - y' * H/2)`.
