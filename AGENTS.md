# TP1 Algoritmos — Battle Zone

## Build & run

```sh
make          # gcc -Wall -Werror -std=c99 -pedantic -g $(sdl2-config --cflags --libs) -lm
make clean    # rm -f *.o battlezone
./battlezone  # requires modelos.stl in CWD
```

- **`-Werror` is on** — any warning = build failure. Fix warnings before building.
- No test framework, no linter, no typecheck.
- Single binary, flat directory, no header-only units.
- `EJ1.c` `EJ2.c` `EJ3.c` are standalone exercises **not compiled by `make`**.

## Architecture

| File | Role |
|------|------|
| `main.c` | SDL init, game loop (events → update → draw → present), FPS pacing. No game or render logic. |
| `juego.c/h` | **TDA juego**: game state (tanks, missiles, obstacles, models list, score), init, events, update (physics, collisions, enemy AI, respawn, score). Exposes read-only accessors for rendering. |
| `dibujo.c/h` | **TDA dibujo**: rendering (camera, world, HUD, crosshair, compass, explosions) with its own transform `pila`. Only presentation logic. |
| `modelo.c/h` | Opaque 3D wireframe model type (copies arrays on create) |
| `matriz.c/h` | Opaque 4×4 matrix algebra (row-major, homogenous coords) |
| `tanque.c/h` | Tank state & movement mechanics (position, heading, turret, lives, cooldown). Movement uses clamped dt for precision. |
| `misil.c/h` | Independent missile entity (position, heading, active flag, 2s lifetime, 24 m/s). Two owned by `juego`. |
| `obstaculo.c/h` | Static obstacle (position + model pointer, not owned) |
| `stl.c/h` | Binary STL reader (cátedra format, not standard STL) |
| `pila.c/h` | Generic `void*` stack (transform matrix stack, internal to `dibujo`) |
| `lista.c/h` | Generic linked list (model catalog in `juego.c`, searched by name) |
| `modelos.stl` | Binary file with all 3D models (TANQUE, TORRETA, RADAR, MISIL, HORIZONTE, MONTANA, LUNA, CUBO1-3, PIRAMIDE1-3, LOGO, letters A-Z, digits 0-9, `*`, `+`, `-`, `#`, RESTO1, RESTO2) |

See `guia.md` for detailed architecture walkthrough.

## Gotchas

- All types are opaque — use accessors, never dereference structs.
- `modelo_crear` copies input arrays; caller may `free` immediately.
- `matriz_crear_mc` ≡ `matriz_crear_mz` (rotation in Z/XY plane).
- `_matriz_crear` is internal allocator (prefix `_` signals internal use).
- STL model named `"MONTANA"` (no S), not `"MONTANAS"`.
- `crear_tanque_enemigo` returns `NULL` if spawn overlaps an obstacle (< 5 units).
- Pila cleanup: must `matriz_destruir(pila_desapilar(stack))` *before* `pila_destruir(stack)` to free the identity matrix at bottom. Pila is owned by `dibujo`, not `juego`.
- M_PI is redefined per-file: `3.14` in `main.c`, `juego.c` and `dibujo.c`, full-precision fallback in `tanque.c` and `misil.c`. Angle math mixes these.
- Game world: x,y ∈ [-150, 150], player at (0,0) heading π/2 (+Y), 4 lives. World bounds only constrain obstacle spawning — tanks are not clamped.
- FPS target: 24 (defined in both `main.c` and `juego.c`). Prints `"Perdiendo cuadros"` if frame overruns.
- Tank-vs-tank and tank-vs-obstacle collision: `dx² + dy² < 36` (radius 6). Missile collision: `dx² + dy² < 9` (radius 3). `crear_tanque_enemigo`'s spawn-overlap check uses `< 5` — two different thresholds.
- Enemy respawns at exactly 50 units from player (in `juego_crear` and `reaparecer_enemigo`).
- Player movement is time-based: each keypress (incl. key-repeat) restarts a 0.5s move/rotate (`tanque_iniciar_movimiento` → `_tiempo`), so holding a key = continuous movement; enemy uses `tanque_iniciar_movimiento_tiempo` with random durations.
- Turret rotation is clamped to ±1 rad (`tanque_girar_torreta`); player + enemy turrets both.
- `juego_modelo(juego, nombre)` accepts model names as string literals; for single-char names pass `(char[]){ch, '\0'}`.
- `juego_crear` takes no arguments (no SDL renderer); it reads `modelos.stl` via relative `fopen("modelos.stl","rb")` and only prints an error if *zero* models load — a wrong CWD silently yields empty pointers.
